#include "psx_manifold.h"

static PsxManifold g_manifolds[CFG_MAX_MANIFOLDS] = { };
static U32 g_manifold_free[CFG_MAX_MANIFOLDS] = { };
static U32 g_manifolds_free_top = 0;
static U32 g_next_manifold = 0;
static U32 total_manifolds = 0;

PsxManifold& manifold_get(Inst manifold) {
    if (manifold >= g_next_manifold) {
        THROW("Manifold: attempt to get invalid manifold");
    }

    return g_manifolds[manifold];
}

PsxManifold& manifold_alloc() {
    Inst manifold;

    if (g_manifolds_free_top > 0) {
        g_manifolds_free_top--;
        manifold = g_manifold_free[g_manifolds_free_top];
    }

    else {
        if (g_next_manifold >= CFG_MAX_MANIFOLDS) {
            THROW("Physics: no more manifolds");
        }

        manifold = g_next_manifold++;
    }


    // instance new manifold at index
    PsxManifold& m = g_manifolds[manifold];

    if (m.in_use) {
        THROW("Physics: got manifold in use @ index=%i", manifold);
    }

    m.index = manifold;
    m.in_use = true;
    m.user_data = nullptr;
    total_manifolds++;

    return m;
}

void manifold_free(Inst manifold) {
    PsxManifold& m = manifold_get(manifold);
        
    if (!m.in_use) {
        return;
    }

    m.in_use = false;
    m.user_data = nullptr;

    if (g_manifolds_free_top >= CFG_MAX_MANIFOLDS) {
        THROW("Physics: no more free slots");
    }

    total_manifolds--;
    g_manifold_free[g_manifolds_free_top++] = m.index;
}

Inst manifold_new(
    Inst collider_a, 
    Inst collider_b, 
    bool colliding,
    Vec2 normal,
    Vec2 tangent,
    Vec2 contact,
    F32  depth
) {
    PsxManifold& manifold = manifold_alloc();

    manifold.collider_a = collider_a;
    manifold.collider_b = collider_b;
    manifold.colliding = colliding;
    manifold.normal = normal;
    manifold.tangent = tangent;
    manifold.depth = depth;
    manifold.contact = contact;

    return manifold.index;
}

void manifold_solve(const PsxManifold& m, F32 dt) {
    if (!m.colliding) return;

    PsxCollider& colA = collider_get(m.collider_a);
    PsxCollider& colB = collider_get(m.collider_b);
    PsxSpacial&  A    = spacial_get(colA.spacial);
    PsxSpacial&  B    = spacial_get(colB.spacial);

    if (!colA.shape || !colB.shape) return;
    if (!A.in_use || !B.in_use) return;

    // response logic
    bool a_static = (A.flags & SPACIAL_FLAG_STATIC);
    bool b_static = (B.flags & SPACIAL_FLAG_STATIC);
    if (a_static && b_static) return;

    // mass
    F32 inv_mass_a = a_static ? 0.f : A.inv_mass;
    F32 inv_mass_b = b_static ? 0.f : B.inv_mass;

    // interia
    F32 inv_i_a    = a_static ? 0.f : A.inv_inertia;
    F32 inv_i_b    = b_static ? 0.f : B.inv_inertia;

    F32 inv_mass_sum = inv_mass_a + inv_mass_b;
    if (inv_mass_sum <= 0.f) return;

    // material properties
    F32 restitution = f32_clamp(
        fmaxf(
            material_get_restitution(colA.material), 
            material_get_restitution(colB.material)
        ), 
        0.f, 1.f);

    F32 friction = sqrtf(
        material_get_friction(colA.material) *  
        material_get_friction(colB.material)
    );

    Vec2 normal = m.normal;   // unit collision normal
    Vec2 contact = m.contact; // contact point

    // --- Positional correction (light, just to avoid sinking)
    {
        const F32 slop = 0.02f;
        const F32 percent = 0.2f;

        F32 depth = m.depth - slop;
        if (depth < 0.f) depth = 0.f;

        Vec2 corr = normal * (depth * percent / inv_mass_sum);

        if (!a_static) A.pos -= corr * inv_mass_a;
        if (!b_static) B.pos += corr * inv_mass_b;
    }

    // compute contact velocities
    Vec2 ra = contact - A.pos;
    Vec2 rb = contact - B.pos;

    Vec2 vel_a = A.vel + vec2_cross(A.ang_vel, ra);
    Vec2 vel_b = B.vel + vec2_cross(B.ang_vel, rb);
    Vec2 rv = vel_b - vel_a; // relative velocity

    // normal impulse
    F32 max_friction;
    do {
        F32 vel_norm = vec2_dot(rv, normal);
        if (vel_norm > 0.f) continue; // separating

        F32 rn_a = vec2_cross(ra, normal);
        F32 rn_b = vec2_cross(rb, normal);

        F32 denom_norm = inv_mass_a + inv_mass_b + rn_a * rn_a * inv_i_a + rn_b * rn_b * inv_i_b;
        if (denom_norm < 0.0005) continue; // will yeet shit into andromeda without this
        
        F32 j = -(1.f + restitution) * vel_norm / denom_norm;
        max_friction = j * friction; // max friction is relative to normal
        Vec2 impulse_norm = normal * j;

        printf("bounce %.4f %.4f\n", vel_norm, denom_norm);
        if (!a_static) spacial_impulse(A, -impulse_norm, contact);
        if (!b_static) spacial_impulse(B,  impulse_norm, contact);
    } while(0);


    // friction impulse
    do {
        Vec2 tangent = rv - normal * vec2_dot(rv, normal);
        F32 tlen = vec2_length(tangent);
        if (tlen < 0.0005) continue;;
        tangent /= tlen;

        F32 vel_tan = vec2_dot(rv, tangent);
        F32 rt_a = vec2_cross(ra, tangent);
        F32 rt_b = vec2_cross(rb, tangent);

        F32 denom_tan = inv_mass_a + inv_mass_b + rt_a * rt_a * inv_i_a + rt_b * rt_b * inv_i_b;
        if (denom_tan < 0.0005) continue;; // will yeet shit onto kepler-B without this

        F32 jt = -vel_tan / denom_tan;
        jt = f32_clamp(jt, -max_friction, +max_friction);

        Vec2 impulse_tan = tangent * jt;

        if (!a_static) spacial_impulse(A, -impulse_tan, contact);
        if (!b_static) spacial_impulse(B,  impulse_tan, contact);
    } while(0);
}

void manifolds_solve(F32 dt) {
    for (Inst i = 0; i < g_next_manifold; ++i) {
        PsxManifold& m = g_manifolds[i];
        if (!m.in_use) continue;

        manifold_solve(m, dt);

        #if !(CFG_MANIFOLDS_RENDER) // need to free manifolds after render pass
        manifold_free(i);
        #endif
    }
}

void manifolds_render() {
    #if CFG_MANIFOLDS_RENDER

    for (Inst i = 0; i < g_next_manifold; ++i) {
        PsxManifold& m = g_manifolds[i];
        if (!m.in_use) continue;

        shape_point(m.contact); // render contact point

        const PsxCollider& collider_a = collider_get(m.collider_a);
        const PsxCollider& collider_b = collider_get(m.collider_b);

        // render normals
        Vec2 pos_a, pos_b;
        if (collider_a.shape == SHAPE_POLY) {
            pos_a = collider_a.poly.center;
        } else {
            pos_a = collider_get_pos(collider_a);
        }

        if (collider_b.shape == SHAPE_POLY) {
            pos_b = collider_b.poly.center;
        } else {
            pos_b = collider_get_pos(collider_b);
        }

        shape_line(pos_a, pos_a + (m.normal * 5));
        shape_line(pos_b, pos_b - (m.normal * 5));

        manifold_free(i);
    }

    #endif
}

bool manifold_get_colliding(Inst manifold) {
    return manifold_get(manifold).colliding;
}

/*
    manifold cases
*/

static Inst manifold_get_poly_poly(const PsxCollider& R1, const PsxCollider& R2) {
    if (R1.shape != SHAPE_POLY || R2.shape != SHAPE_POLY) {
        return NO_INSTANCE;
    }

    if (R1.poly.count < 2 || R2.poly.count < 2) {
        return NO_INSTANCE;
    }

    Vec2 normal = {0, 0};
    F32 depth = FLT_MAX;

    // poly info
    const Vec2* poly_a       = R1.poly.transform;
    U32         poly_a_count = R1.poly.count;
    const Vec2* poly_b       = R2.poly.transform;
    U32         poly_b_count = R2.poly.count;

    if (!algo_separate_axis(poly_a, poly_a_count, poly_b, poly_b_count, normal, depth)) { return NO_INSTANCE; }
    if (!algo_separate_axis(poly_b, poly_b_count, poly_a, poly_a_count, normal, depth)) { return NO_INSTANCE; }

    bool colliding = true;
    Vec2 dir = R2.poly.center - R1.poly.center;
    if (vec2_dot(normal, dir) < 0.f) normal = -normal;
    Vec2 tangent = vec2_perp(normal);
    Vec2 contact{};

    if (collider_get_flags(R1, SPACIAL_FLAG_RIGID) || 
        collider_get_flags(R2, SPACIAL_FLAG_RIGID)
    ) {
        contact = algo_get_contact_point(
            poly_a, poly_a_count, 
            poly_b, poly_b_count, 
            normal
        );
    }

    return manifold_new(
        R1.id, R2.id, 
        colliding, 
        normal, 
        tangent, 
        contact, 
        depth
    );
}

static Inst manifold_get_poly_circle(const PsxCollider& R, const PsxCollider& C) {
    if (R.shape != SHAPE_POLY || C.shape != SHAPE_CIRCLE) {
        return NO_INSTANCE;
    }

    const Vec2* poly  = R.poly.transform;
    const U32   count = R.poly.count;
    const Vec2  C_pos = collider_get_pos(C);
    const Vec2  R_pos = collider_get_pos(R);

    F32 best_overlap = FLT_MAX;
    Vec2 best_axis   = { 0.f, 0.f };
    Vec2 axis;

    for (U32 i = 0; i < count; ++i) {
        U32 j = (i + 1) % count;

        Vec2 edge = poly[j] - poly[i];
        axis = vec2_perp(edge);

        if (axis.x == 0.f && axis.y == 0.f) {
            continue;
        }

        axis = vec2_normal(axis);

        F32 minP, maxP;
        algo_project_1d(poly, count, axis, minP, maxP);

        F32 cproj = vec2_dot(C_pos, axis);
        F32 minC  = cproj - C.circ.radius;
        F32 maxC  = cproj + C.circ.radius;

        // no overlap on this axis -> separating axis -> no collision
        if (!algo_overlap_1d(minP, maxP, minC, maxC)) {
            return NO_INSTANCE;
        }

        // compute overlap on this axis
        F32 overlap = fminf(maxP, maxC) - fmaxf(minP, minC);
        if (overlap < best_overlap) {
            best_overlap = overlap;
            best_axis    = axis;
        }
    }

    U32 closest = 0;
    F32 best_dist_sq = vec2_length_sq(poly[0] - C_pos);

    for (U32 i = 1; i < count; ++i) {
        F32 dist_sq = vec2_length_sq(poly[i] - C_pos);
        if (dist_sq < best_dist_sq) {
            best_dist_sq = dist_sq;
            closest      = i;
        }
    }

    axis = poly[closest] - C_pos;
    if (!(axis.x == 0.f && axis.y == 0.f)) {
        axis = vec2_normal(axis);

        F32 minP, maxP;
        algo_project_1d(poly, count, axis, minP, maxP);

        F32 cproj = vec2_dot(C_pos, axis);
        F32 minC  = cproj - C.circ.radius;
        F32 maxC  = cproj + C.circ.radius;

        if (!algo_overlap_1d(minP, maxP, minC, maxC)) {
            return NO_INSTANCE;
        }

        F32 overlap = fminf(maxP, maxC) - fmaxf(minP, minC);
        if (overlap < best_overlap) {
            best_overlap = overlap;
            best_axis    = axis;
        }
    }

    if (best_axis.x == 0.f && best_axis.y == 0.f) {
        Vec2 dir = C_pos - R_pos;
        best_axis = vec2_normal(dir, {1, 0});
        best_overlap = C.circ.radius;
    }

    Vec2 center_delta = C_pos - R_pos;
    if (vec2_dot(center_delta, best_axis) < 0.f) {
        best_axis = -best_axis;
    }

    // manifold data
    Vec2 normal  = best_axis;
    Vec2 tangent = { -normal.y, normal.x };
    F32  depth   = best_overlap;
    Vec2 contact = (-normal * C.circ.radius) + C_pos;

    bool colliding = true;
    return manifold_new(
        R.id, C.id, 
        colliding, 
        normal, 
        tangent, 
        contact, 
        depth
    );
}

static Inst manifold_get_circ_circ(const PsxCollider& CA, const PsxCollider& CB) {
    if (CA.shape != SHAPE_CIRCLE || CB.shape != SHAPE_CIRCLE) {
        return NO_INSTANCE;
    }
    
    Vec2 p1 = collider_get_pos(CA);
    Vec2 p2 = collider_get_pos(CB);

    Vec2 diff = p2 - p1;
    F32 tot_rad = CA.circ.radius + CB.circ.radius;
    F32 tot_rad_sq = tot_rad * tot_rad;
    F32 tot_dis_sq = vec2_length_sq(diff);

    bool colliding = tot_dis_sq < tot_rad_sq;
    if (!colliding) { return NO_INSTANCE; }

    // manifold data
    F32 dist = sqrtf(tot_dis_sq);
    Vec2 normal = vec2_normal(diff, dist, { 1, 0 });
    Vec2 tangent = vec2_perp(normal);
    F32 depth = tot_rad - dist;
    Vec2 contact = p1 + normal * CA.circ.radius;

    return manifold_new( 
        CA.id, CB.id, 
        colliding, 
        normal, 
        tangent, 
        contact, 
        depth 
    );
}

/*
    get a manifold
*/

Inst manifold_generate(U32 collider_a, U32 collider_b) {
    PsxCollider& ca = collider_get(collider_a);
    const PsxCollider& cb = collider_get(collider_b);
    Inst manifold = NO_INSTANCE;

    if ((ca.shape == SHAPE_POLY && cb.shape == SHAPE_CIRCLE)) {
        manifold = manifold_get_poly_circle(ca, cb);
    }

    else if ((ca.shape == SHAPE_CIRCLE && cb.shape == SHAPE_POLY)) {
        manifold = manifold_get_poly_circle(cb, ca);
    }

    else if ((ca.shape | cb.shape) == SHAPE_CIRCLE) {
        manifold = manifold_get_circ_circ(ca, cb);
    }

    else if ((ca.shape | cb.shape) == SHAPE_POLY) {
        manifold = manifold_get_poly_poly(ca, cb);
    }


    return manifold;
}

U32 count_manifolds() {
    return total_manifolds;
}