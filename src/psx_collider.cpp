#include "psx_collider.h"
#include "analytics.h"

static PsxCollider g_colliders[CFG_MAX_COLLIDERS] = { };
static U32 g_updated_colliders[CFG_MAX_COLLIDERS] = { };
static U32 g_collider_free[CFG_MAX_COLLIDERS] = { };
static U32 g_colliders_free_top = 0;
static U32 g_next_collider = 0;
static U32 g_updated_collider_count = 0;

PsxCollider& collider_get(U32 index) {
    if (index >= g_next_collider) {
        THROW("Collider: attempt to get invalid collider");
    }

    return g_colliders[index];
}

PsxCollider& collider_alloc() {
    U32 index;

    if (g_colliders_free_top > 0) {
        g_colliders_free_top--;
        index = g_collider_free[g_colliders_free_top];
    }

    else {
        if (g_next_collider >= CFG_MAX_COLLIDERS) {
            THROW("Physics: no more colliders");
        }

        index = g_next_collider++;
    }

    // instance new collider at index
    PsxCollider& collider = g_colliders[index];

    if (collider.shape != SHAPE_NONE) {
        THROW("Physics: got collider in use @ index=%i type=%i", index, collider.shape);
    }

    collider.phase = COLLIDER_PHASE_NONE;
    collider.id = index;
    collider.shape = SHAPE_NONE;
    collider.user_data = nullptr;

    return collider;
}

void collider_make_heap_buffer(PsxCollider& collider, U32 size) {
    collider.heap_buffer = (U8*) malloc(size);
    collider.alloc_bytes = size;
}

void collider_free(U32 index) {
    PsxCollider& collider = collider_get(index);
        
    if (collider.shape == SHAPE_NONE) {
        return;
    }

    collider.shape = SHAPE_NONE;
    collider.user_data = nullptr;

    if (g_colliders_free_top >= CFG_MAX_COLLIDERS) {
        THROW("Physics: no more free slots");
    }

    if (collider.alloc_bytes > 0) {
        free(collider.heap_buffer);
    }
    collider.alloc_bytes = 0;

    g_collider_free[g_colliders_free_top++] = collider.id;
}

Inst collider_new_circle(F32 radius, PsxColliderConfig cfg) {
    PsxCollider& collider = collider_alloc();
    
    // base collider
    collider.shape = SHAPE_CIRCLE;
    collider.circ.radius = radius;
    collider.spacial = cfg.spacial;
    collider.material = cfg.material;
    collider.offset = cfg.offset;
    collider.user_data = cfg.user_data;

    collider.bounding_box = {{ F32_MAX, F32_MAX }, { -F32_MAX, -F32_MAX }};

    return collider.id;
}

Inst collider_new_poly(GlxPolygon identity, F32 scale, PsxColliderConfig cfg) {
    PsxCollider& collider = collider_alloc();
    
    // base collider
    collider.shape = SHAPE_POLY;
    collider.spacial = cfg.spacial;
    collider.material = cfg.material;
    collider.offset = cfg.offset;
    collider.user_data = cfg.user_data;

    collider.bounding_box = {{ F32_MAX, F32_MAX }, { -F32_MAX, -F32_MAX }};

    // store the identity & transform matrix
    collider_make_heap_buffer(collider, 2 * identity.bytes);

    // collider_make_heap_buffer(collider, matrix_size + 1);

    collider.poly.identity  = (Vec2*)  collider.heap_buffer;
    collider.poly.transform = (Vec2*) (collider.heap_buffer + identity.bytes);

    memcpy(collider.poly.identity, identity.data, identity.count * sizeof(Vec2));
    
    collider.poly.scale = scale;
    collider.poly.count = identity.count;
    collider.poly.center = { 0, 0 };

    // initial transform
    glx_transform_poly_2d(
        collider_get_pos(collider.id), 
        collider.poly.transform, 
        collider.poly.identity, 
        collider.poly.count, 
        collider.poly.scale,
        spacial_get_ang(collider.spacial),
        &collider.bounding_box,
        &collider.poly.center
    );

    return collider.id;
}

Inst collider_new_rect(Vec2 area, PsxColliderConfig cfg) {
    GlxPolygon identity = GlxPolygon({
        {-area.w * 0.5f, -area.h * 0.5f},
        {-area.w * 0.5f,  area.h * 0.5f},
        { area.w * 0.5f,  area.h * 0.5f},
        { area.w * 0.5f, -area.h * 0.5f},
    });

    return collider_new_poly(identity, 1.f, cfg);
}

Vec2 collider_get_pos(const PsxCollider& c) {
    PsxSpacial& s = spacial_get(c.spacial);
    Vec2 pos = (s.pos + c.offset);
    if (s.ang == 0.f) { return pos; }
    return vec2_rotate(pos, s.pos, s.ang);
}

U32 collider_get_group(const PsxCollider& c) {
    return spacial_get(c.spacial).group;
}

U32 collider_get_layer(const PsxCollider& c) {
    return spacial_get(c.spacial).layer;
}

U32 collider_get_flags(const PsxCollider& c) {
    return spacial_get(c.spacial).flags;
}

U32 collider_get_flags(const PsxCollider& c, U32 flag) {
    return spacial_get(c.spacial).flags & flag;
}

bool collider_compare_groups(const PsxCollider& a, const PsxCollider& b) {
    U32 ga = collider_get_group(a);
    U32 gb = collider_get_group(b);

    return ga == gb;
}

bool collider_compare_layer(const PsxCollider& a, const PsxCollider& b) {
    U32 la = collider_get_layer(a);
    U32 lb = collider_get_layer(b);

    return la == lb;
}


Vec2 collider_get_pos(Inst collider) {
    return collider_get_pos(collider_get(collider));
}

F32 collider_get_radius(Inst index) {
    const PsxCollider& collider = collider_get(index);

    if (collider.shape == SHAPE_CIRCLE) {
        return collider.circ.radius;
    } else {
        return 0.f;
    }
}

const AABB& collider_get_bounding_box(Inst collider) {
    const PsxCollider& c = collider_get(collider);
    return c.bounding_box;
}

void collider_draw(Inst collider) {
    PsxCollider& c = collider_get(collider);
    Vec2 pos = collider_get_pos(collider);

    Color prev_color = shape_get_color();
    GLXDrawMode prev_draw_mode = shape_get_draw_mode();

    if (c.phase & COLLIDER_PHASE_RESOLVE) { // blue when resoliving
        shape_color({64, 64, 255, 128});
    }
    else if (c.phase & COLLIDER_PHASE_NARROW) { // red when checking
        shape_color({255, 64, 64, 128});
    } 
    else { // green when not colliding
        shape_color({64, 255, 64, 128});
    }

    switch (c.shape) {
        case (SHAPE_CIRCLE) : { 
            shape_point(pos);
            shape_circle(pos, c.circ.radius); 
            break;
        }
        case (SHAPE_POLY) : { 
            shape_point(c.poly.center);
            shape_polygon(c.poly.transform, c.poly.count); 
            break;
        }
        default : { break; }
    }

    #if CFG_RENDER_BOUNDING_BOX

    shape_draw_lines();

    shape_bounding_box(c.bounding_box);

    #endif

    shape_color(prev_color);
    shape_draw_mode(prev_draw_mode);
}

//

void collider_draw_all() {
    shape_draw_lines();

    /*
        render test to visualize BVH
    */



    for (int i = 0; i < g_next_collider; ++i) {
        PsxCollider& fc = collider_get(i);

        #if CFG_FILL_ON_COLLIDE

        if (fc.phase & COLLIDER_PHASE_RESOLVE) {
            shape_draw_fill();
        } else {
            shape_draw_lines();
        }

        #endif

        collider_draw(i);
    }
}

void collider_filter_updated() {
    g_updated_collider_count = 0;
    for (int i = 0; i < g_next_collider; ++i) {

        // run sanity check on collider
        PsxCollider& c = collider_get(i);
        if (c.shape == SHAPE_NONE) continue;
        if (c.spacial == NO_INSTANCE) continue;
        PsxSpacial& s = spacial_get(c.spacial);
        if (!s.in_use) continue;

        c.phase = COLLIDER_PHASE_BROAD;
        collider_update(i); // run update

        g_updated_colliders[g_updated_collider_count++] = i;

    }
}

void collider_build_bvh() {
    bvh_build(g_updated_colliders, g_updated_collider_count); // construct BVH
}

void collider_update(Inst collider) {
    PsxCollider& c = collider_get(collider);
    if (c.shape == SHAPE_NONE) return;

    PsxSpacial& s = spacial_get(c.spacial);
    if (!s.in_use) return;
    if (s.flags & SPACIAL_FLAG_STATIC) return;

    Vec2 pos = collider_get_pos(collider);

    // recompute polygon
    if (c.shape == SHAPE_POLY) {
        c.bounding_box = {
            { F32_MAX, F32_MAX }, 
            { -F32_MAX, -F32_MAX }
        };

        glx_transform_poly_2d(
            pos, 
            c.poly.transform, 
            c.poly.identity, 
            c.poly.count, 
            c.poly.scale,
            spacial_get_ang(c.spacial),
            &c.bounding_box,
            &c.poly.center
        );

        for (int i = 0; i < c.poly.count; ++i) {
            const Vec2 p = c.poly.transform[i];

            if (p.x < c.bounding_box.min.x) c.bounding_box.min.x = p.x;
            if (p.y < c.bounding_box.min.y) c.bounding_box.min.y = p.y;
            if (p.x > c.bounding_box.max.x) c.bounding_box.max.x = p.x;
            if (p.y > c.bounding_box.max.y) c.bounding_box.max.y = p.y;
        }
    }

    if (c.shape == SHAPE_CIRCLE) {
        c.bounding_box.min = pos - c.circ.radius;
        c.bounding_box.max = pos + c.circ.radius;
    }
}

U32 count_colliders() {
    return g_updated_collider_count;
}