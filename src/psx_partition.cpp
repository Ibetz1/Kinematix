#include "psx_partition.h"
#include "analytics.h"

static BvhNode g_bvh_nodes[CFG_MAX_COLLIDERS * 2];
static U32 g_bvh_root = NO_INSTANCE;
static U32 g_bvh_free_head = NO_INSTANCE;
static U32 g_bvh_node_count = 0;

bool bvh_is_leaf(const BvhNode& node) { 
    return  node.child1 == NO_INSTANCE; 
};

Inst bvh_new_node() {
    Inst id = g_bvh_node_count++;

    BvhNode& node = g_bvh_nodes[id];
    node.parent   = NO_INSTANCE;
    node.child1   = NO_INSTANCE;
    node.child2   = NO_INSTANCE;
    node.collider = NO_INSTANCE;

    return id;
}

BvhNode& bvh_get_node(Inst id) {
    if (id > g_bvh_node_count) {
        THROW("attempt to get invalid node in BVH");
    }

    return g_bvh_nodes[id];
}

void bvh_set_node_parent(Inst id, Inst parent) {
    bvh_get_node(id).parent = parent;
}

const AABB& bvh_get_node_box(Inst id) {
    return bvh_get_node(id).box;
}

U32 bvh_partition_ids(Inst* ids, U32 count, Axis axis, F32 split) {
    U32 left  = 0;
    U32 right = count;

    while (left < right) {
        Inst id = ids[left];
        const AABB& box = collider_get_bounding_box(id);

        Vec2 center = glx_aabb_center(box);
        F32 val = (axis == AXIS_X) ? center.x : center.y;

        if (val < split) {
            ++left;
        } else {
            --right;
            Inst tmp   = ids[left];
            ids[left]  = ids[right];
            ids[right] = tmp;
        }

    }

    // If everything ended up on one side, force a split
    if (left == 0 || left == count) {
        left = count / 2;
    }

    return left;
}

Inst bvh_build_recursive(Inst* ids, U32 count) {
    if (count == 0) return NO_INSTANCE;

    // base reqs
    Inst first = ids[0];
    Inst node_id = bvh_new_node();
    BvhNode& node = bvh_get_node(node_id);

    if (count == 1) {
        node.collider = first;
        node.box = collider_get_bounding_box(first);
        return node_id;
    }

    // get box containing all colliders
    AABB combined = collider_get_bounding_box(first);
    for (U32 i = 1; i < count; ++i) {
        combined = glx_aabb_merge(combined, collider_get_bounding_box(i));
    }

    F32 dx = combined.max.x - combined.min.x;
    F32 dy = combined.max.y - combined.min.y;
    Axis axis = (dx > dy) ? AXIS_X : AXIS_Y;

    F32 split = (axis == AXIS_X) 
        ? (combined.min.x + combined.max.x) * 0.5f 
        : (combined.min.y + combined.max.y) * 0.5f;

    /*
        split IDS by slice
    */

    U32 left_count = bvh_partition_ids(ids, count, axis, split);
    U32 right_count = count - left_count;

    Inst* left_ids = ids;
    Inst* right_ids = ids + left_count;

    node.child1 = bvh_build_recursive(left_ids, left_count);
    node.child2 = bvh_build_recursive(right_ids, right_count);
    node.collider = NO_INSTANCE;

    if (node.child1 != NO_INSTANCE) bvh_set_node_parent(node.child1, node_id);
    if (node.child2 != NO_INSTANCE) bvh_set_node_parent(node.child2, node_id);

    node.box = glx_aabb_merge(
        bvh_get_node_box(node.child1),
        bvh_get_node_box(node.child2)
    );

    return node_id;
}

void bvh_build(Inst* colliders, U32 count) {
    g_bvh_node_count = 0;

    if (count == 0) {
        g_bvh_root = NO_INSTANCE;
        return;
    }

    g_bvh_root = bvh_build_recursive(
        colliders,
        count
    );
}

void bvh_render_node(U32 node_id) {
    if (node_id == NO_INSTANCE) return;
    BvhNode& n = g_bvh_nodes[node_id];

    shape_bounding_box(n.box);

    if (bvh_is_leaf(n)) return;

    if (n.child1 != NO_INSTANCE) bvh_render_node(n.child1);
    if (n.child2 != NO_INSTANCE) bvh_render_node(n.child2);
}

void bvh_render() {
    #if CFG_RENDER_BVH

    if (g_bvh_root != NO_INSTANCE) {
        bvh_render_node(g_bvh_root);
    }

    #endif
}

void bvh_calculate_manifolds() {
    if (g_bvh_root == NO_INSTANCE) {
        return;
    }

    struct NodePair { U32 a; U32 b; };

    static NodePair stack[CFG_MAX_COLLIDERS * 4];
    U32 stack_top = 0;

    stack[stack_top++] = { g_bvh_root, g_bvh_root };

    while (stack_top > 0) {
        NodePair pair = stack[--stack_top];
        U32 na = pair.a;
        U32 nb = pair.b;

        if (na == NO_INSTANCE || nb == NO_INSTANCE) {
            continue;
        }

        BvhNode& A = g_bvh_nodes[na];
        BvhNode& B = g_bvh_nodes[nb];

        // When same node, split it into its children combinations
        if (na == nb) {
            if (!bvh_is_leaf(A)) {
                if (A.child1 != NO_INSTANCE) {
                    stack[stack_top++] = { A.child1, A.child1 };
                    stack[stack_top++] = { A.child1, A.child2 };
                }
                if (A.child2 != NO_INSTANCE) {
                    stack[stack_top++] = { A.child2, A.child2 };
                }
            }
            continue;
        }

        if (!glx_aabb_check(A.box, B.box)) {
            continue;
        }

        bool leafA = bvh_is_leaf(A);
        bool leafB = bvh_is_leaf(B);

        if (leafA && leafB) {
            U32 a_id = A.collider;
            U32 b_id = B.collider;
            if (a_id == b_id) continue;

            PsxCollider& ca = collider_get(a_id);
            PsxCollider& cb = collider_get(b_id);

            if (ca.shape == SHAPE_NONE || cb.shape == SHAPE_NONE) continue;
            if (!collider_compare_layer(ca, cb)) continue;

            if (ca.spacial == cb.spacial)   continue;

            if (!glx_aabb_check(ca.bounding_box, cb.bounding_box)) {
                continue;
            }
            
            ca.phase |= COLLIDER_PHASE_NARROW;
            cb.phase |= COLLIDER_PHASE_NARROW;
            
            Inst manifold = manifold_generate(a_id, b_id);

            if (manifold != NO_INSTANCE) {
                ca.phase |= COLLIDER_PHASE_RESOLVE;
                cb.phase |= COLLIDER_PHASE_RESOLVE;

            }
        } else {
            // At least one is internal: split whichever is "bigger" or not a leaf
            if (!leafA && (leafB || glx_aabb_perimeter(A.box) >= glx_aabb_perimeter(B.box))) {
                // Expand A against B
                if (A.child1 != NO_INSTANCE) stack[stack_top++] = { A.child1, nb };
                if (A.child2 != NO_INSTANCE) stack[stack_top++] = { A.child2, nb };
            } else {
                // Expand B against A
                if (B.child1 != NO_INSTANCE) stack[stack_top++] = { na, B.child1 };
                if (B.child2 != NO_INSTANCE) stack[stack_top++] = { na, B.child2 };
            }
        }
    }
}

PsxRayResult bvh_cast_ray(
    const PsxRay& ray,
    bool search_groups,
    bool search_layer
) {
    if (g_bvh_root == NO_INSTANCE) 
        return {};

    struct StackEntry { U32 node; F32 tnear; };
    StackEntry stack[CFG_MAX_COLLIDERS * 4];
    U32 top = 0;

    stack[top++] = { g_bvh_root, 0.f };

    bool hit = false;
    F32 best_t = ray.max_dist;
    Vec2 best_normal{};
    U32 best_collider = NO_INSTANCE;

    while (top > 0) {
        StackEntry e = stack[--top];
        U32 nid = e.node;

        if (nid == NO_INSTANCE) continue;

        BvhNode& N = g_bvh_nodes[nid];

        F32 tnear, tfar;
        if (!ray_check_aabb(ray, N.box, tnear, tfar)) {
            continue;
        }

        if (tnear > best_t)
            continue;

        if (bvh_is_leaf(N)) {
            U32 cid = N.collider;
            PsxCollider& c = collider_get(cid);

            // cull unwanted groups/layers
            if (search_groups && !(collider_get_group(c) & ray.group))
                continue;

            if (search_layer && collider_get_layer(c) != ray.layer)
                continue;

            // run narrow phase
            F32 t;
            Vec2 normal;

            bool local_hit = ray_test_collider(ray, c.id, t, normal);
            if (local_hit && t < best_t && t >= 0.f) {
                best_t = t;
                best_normal = normal;
                best_collider = cid;
                hit = true;
            }
        }
        else {
            if (N.child1 != NO_INSTANCE)
                stack[top++] = { N.child1, tnear };

            if (N.child2 != NO_INSTANCE)
                stack[top++] = { N.child2, tnear };
        }
    }

    PsxRayResult out_hit{};
    out_hit.touched      = hit;
    out_hit.dist     = best_t;
    out_hit.point    = ray.origin + ray.dir * best_t;
    out_hit.normal   = best_normal;
    out_hit.collider = best_collider;
    return out_hit;
}