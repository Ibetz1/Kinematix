#ifndef _PSX_ALGO_H
#define _PSX_ALGO_H

#include "main.h"
#include "vector.h"

/*
    manifold helpers
*/

void algo_project_1d(const Vec2* vertices, U32 count, const Vec2& axis, F32& min, F32& max);

bool algo_overlap_1d(F32 min1, F32 max1, F32 min2, F32 max2);

bool algo_separate_axis(
    const Vec2* poly_a, U32 poly_a_count, 
    const Vec2* poly_b, U32 poly_b_count, 
    Vec2& best_axis, 
    F32& best_depth
);

U32 algo_find_segment(
    const Vec2* poly,
    const U32 count,
    const Vec2& n,
    Vec2* out
);

bool algo_plane_contains_point(const Vec2* plane, const Vec2& point);

bool algo_plane_intersection(
    const Vec2& p1_a,
    const Vec2& p2_a,
    const Vec2& p1_b,
    const Vec2& p2_b,
    F32& distance,
    Vec2& normal
);

// returns number of contact point (1 or 2)
// moves contact points to out
Vec2 algo_get_contact_point(
    const Vec2* poly_a, U32 poly_a_count, 
    const Vec2* poly_b, U32 poly_b_count,
    const Vec2& normal
);

#endif