#ifndef _PSX_RAY_H
#define _PSX_RAY_H

#include "main.h"
#include "vector.h"

#include "psx_algo.h"
#include "gl_express_shape.h"
#include "psx_collider.h"

struct PsxRay {
    Vec2 origin;
    Vec2 dir;
    F32 max_dist;
    U32 group = NO_INSTANCE;
    U32 layer = NO_INSTANCE;
};

struct PsxRayResult {
    bool touched;
    F32 dist;
    Vec2 point;
    Vec2 normal;
    Inst collider;
};

bool ray_check_circle(
    const PsxRay& ray, 
    const Vec2& circ_center, 
    F32 circ_radius, 
    F32& dist_out, 
    Vec2& normal_out
);

bool ray_check_poly(
    const PsxRay& ray, 
    const Vec2* vertices, 
    U32 count, 
    F32& dist_out, 
    Vec2& normal_out
);

bool ray_check_aabb(
    const PsxRay& ray,
    const AABB& box,
    F32& tnear,
    F32& tfar
);

bool ray_test_collider(
    const PsxRay& ray,
    Inst collider,
    F32& out_dist,
    Vec2& out_normal
);

PsxRayResult ray_cast(const PsxRay& ray); // use bvh to cast ray

#endif