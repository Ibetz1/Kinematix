#include "psx_ray.h"

bool ray_check_circle(
    const PsxRay& ray, 
    const Vec2& circ_center, 
    F32 circ_radius, 
    F32& dist_out, 
    Vec2& normal_out
) {
    Vec2 m = ray.origin - circ_center;

    F32 b = vec2_dot(m, ray.dir);
    F32 c = vec2_dot(m, m) - circ_radius * circ_radius;

    if (c > 0.f && b > 0.f) { return false; }

    F32 disc = b * b - c;
    if (disc < 0.f) { return false; }

    F32 sqrt_disc = sqrtf(disc);

    F32 dist = -b - sqrt_disc;

    if (dist < 0.f) {
        dist = -b + sqrt_disc;
        if (dist < 0.f) {
            return false;
        }
    }

    if (dist > ray.max_dist) { return false; }

    dist_out = dist;
    Vec2 hit = ray.origin + ray.dir * dist;
    normal_out = vec2_normal(hit - circ_center);

    return true;
}

bool ray_check_poly(
    const PsxRay& ray, 
    const Vec2* vertices, 
    U32 count, 
    F32& dist_out, 
    Vec2& normal_out
) {
    if (count < 3) {
        return false;
    }

    bool hit = false;

    F32 best_t = 1.0f;
    Vec2 best_norm{};

    const Vec2 r1 = ray.origin;
    const Vec2 r2 = ray.origin + ray.dir * ray.max_dist;

    for (U32 i = 0; i < count; ++i) {
        const Vec2& p1 = vertices[i];
        const Vec2& p2 = vertices[(i + 1) % count];

        F32 t = 0.f;
        Vec2 n{};
        if (algo_plane_intersection(p1, p2, r1, r2, t, n)) {
            if (t >= 0.f && t < best_t) {
                best_t   = t;
                best_norm = n;
                hit = true;
            }
        }
    }

    if (!hit) { return false; }

    dist_out   = best_t * ray.max_dist;
    normal_out = best_norm;
    return true;
}

bool ray_check_aabb(
    const PsxRay& ray,
    const AABB& box,
    F32& tnear,
    F32& tfar
) {
    tnear = -FLT_MAX;
    tfar  =  FLT_MAX;

    for (int axis = 0; axis < 2; ++axis) {
        F32 origin = (axis == 0) ? ray.origin.x : ray.origin.y;
        F32 dir    = (axis == 0) ? ray.dir.x    : ray.dir.y;
        F32 minB   = (axis == 0) ? box.min.x    : box.min.y;
        F32 maxB   = (axis == 0) ? box.max.x    : box.max.y;

        if (fabsf(dir) < 1e-8f) {
            if (origin < minB || origin > maxB) {
                return false;
            }
            continue;
        }

        F32 t1 = (minB - origin) / dir;
        F32 t2 = (maxB - origin) / dir;

        if (t1 > t2) {
            F32 tmp = t1;
            t1 = t2;
            t2 = tmp;
        }

        if (t1 > tnear) tnear = t1;
        if (t2 < tfar)  tfar  = t2;

        if (tnear > tfar) return false;
        if (tfar < 0.f)   return false;
    }

    return true;
}

bool ray_test_collider(const PsxRay& ray, Inst collider, F32& out_dist, Vec2& out_normal) {
    const PsxCollider& c = collider_get(collider);

    switch (c.shape) {
        case SHAPE_CIRCLE:
            return ray_check_circle(
                ray,
                collider_get_pos(c),
                c.circ.radius,
                out_dist,
                out_normal
            );

        case SHAPE_POLY:
            return ray_check_poly(
                ray,
                c.poly.transform,
                c.poly.count,
                out_dist,
                out_normal
            );

        default:
            return false;
    }

    return false;
}

PsxRayResult ray_cast(const PsxRay& ray) {
    return bvh_cast_ray(ray, ray.group != NO_INSTANCE, ray.layer != NO_INSTANCE);
}