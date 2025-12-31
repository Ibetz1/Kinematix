#include "psx_algo.h"

void algo_project_1d(const Vec2* vertices, U32 count, const Vec2& axis, F32& min, F32& max) {
    F32 d = vec2_dot(vertices[0], axis);
    min = d;
    max = d;

    for (int i = 1; i < count; ++i) {
        d = vec2_dot(vertices[i], axis);
        if (d < min) min = d;
        if (d > max) max = d;
    }
}

bool algo_overlap_1d(F32 min1, F32 max1, F32 min2, F32 max2) {
    return !(max1 < min2 || max2 < min1);
}

bool algo_separate_axis(
    const Vec2* poly_a, U32 poly_a_count, 
    const Vec2* poly_b, U32 poly_b_count, 
    Vec2& best_axis, 
    F32& best_depth
) {
    for (U32 i = 0; i < poly_a_count; ++i) {
        U32 j = (i + 1) % poly_a_count;

        Vec2 edge = poly_a[j] - poly_a[i];
        Vec2 axis = vec2_perp(edge);

        F32 len_sq = vec2_length_sq(axis);
        if (len_sq == 0.f) continue;

        axis = vec2_normal(axis, sqrtf(len_sq));

        F32 min_a, max_a, min_b, max_b;

        algo_project_1d(poly_a, poly_a_count, axis, min_a, max_a);
        algo_project_1d(poly_b, poly_b_count, axis, min_b, max_b);

        if (!algo_overlap_1d(min_a, max_a, min_b, max_b)) {
            return false;
        }

        F32 overlap = fminf(max_a, max_b) - fmaxf(min_a, min_b);

        if (overlap < best_depth) {
            best_depth = overlap;
            best_axis = axis;
        }
    }

    return true; // all axes overlapped
}

U32 algo_find_segment(
    const Vec2* poly,
    const U32 count,
    const Vec2& n,
    Vec2* out
) {
    constexpr U32 axis_size = 2;

    F32 best_proj = -FLT_MAX;
    Vec2 a{}, b{};

    for (U32 i = 0; i < count; ++i) {
        U32 j = (i + 1) % count;

        Vec2 p1 = poly[i], p2 = poly[j];
        Vec2 mid = (p1 + p2) * 0.5f;
        F32 proj = vec2_dot(mid, n);

        if (proj > best_proj) {
            best_proj = proj;
            a = p1, b = p2;
        }
    }

    out[0] = a, out[1] = b;
    return axis_size; // always returns 2 vertices
}

// breaks on large planes
bool algo_plane_contains_point(const Vec2* plane, const Vec2& point) {
    constexpr F32 eps = 0.25f;  // max deveviation from plane
    constexpr F32 eps_sq = eps * eps;

    const Vec2 a = plane[0], b = plane[1];
    const Vec2 ab = {     b.x - a.x,     b.y - a.y };
    const Vec2 ap = { point.x - a.x, point.y - a.y };

    F32 len_sq = vec2_length_sq(ab), dist_sq{};
    if (len_sq == 0.0f) return vec2_length_sq(ap) < eps_sq;

    F32 cross = vec2_cross(ab, ap);
    return (cross * cross) / len_sq < eps_sq;
}

// returns number of contact point (1 or 2)
// moves contact points to out
Vec2 algo_get_contact_point(
    const Vec2* poly_a, U32 poly_a_count, 
    const Vec2* poly_b, U32 poly_b_count,
    const Vec2& normal
) {
    
    /*
        a plane consists of at least 1 axis, and addition vertices
        along another axis that fall along the primary axis
        
        plane[0] & plane[1] are initially defined as the planes axis
        plane[2] & plane[3] are potentially extended points along that axis
    */
    static constexpr U32 mpv = 4; // a plane will always consist of at most 4 vertices
    static constexpr U32 acn = 2; // an axis will always contain exactly 2 points
   
    
    // identify possible collision planes/axis
    Vec2 pa[mpv]{}, pb[mpv]{};
    U32 pa_cnt = algo_find_segment(poly_a, poly_a_count,  normal, pa);
    U32 pb_cnt = algo_find_segment(poly_b, poly_b_count, -normal, pb);

    for (int i = 0; i < acn; ++i) {
        if (algo_plane_contains_point(pa, pb[i])) { pa[pa_cnt++] = pb[i]; }
        if (algo_plane_contains_point(pb, pa[i])) { pb[pb_cnt++] = pa[i]; }
    }

    // axis & plane of collision
    bool use_a  =      (pa_cnt >= pb_cnt);
    Vec2* plane =       use_a ? pa     : pb;
    U32   plane_count = use_a ? pa_cnt : pb_cnt;

    if (plane_count > mpv)  return {}; // this is degenerate, never more than 4 vertices
    if (plane_count <= acn) return {}; // a plane needs an axis and an vertex or two

    F32 min_prj =  FLT_MAX, 
        max_prj = -FLT_MAX;
        
    const Vec2* axis     = plane; // axis is first two indices on the plane
    const Vec2  axis_dir = axis[1] - axis[0];

    for (int i = 0; i < plane_count; ++i) {

        // project plane point onto axis
        F32 prj = vec2_dot(plane[i] - axis[0], axis_dir);
        
        // find extreme projections, anything between these is a collision point
        min_prj = fminf(prj, min_prj);
        max_prj = fmaxf(prj, max_prj);
    }

    U32 count{};
    Vec2 contact{};

    for (int i = 0; i < plane_count; ++i) {
        
        // project plane point onto axis
        F32 prj = vec2_dot(plane[i] - axis[0], axis_dir);

        // use as collision point, it has minimum two neighbors
        if (prj > min_prj && prj < max_prj) {
            contact += plane[i];
            ++count;
        }
    }

    contact /= (F32) count;

    return contact;
}

bool algo_plane_intersection(
    const Vec2& p1_a,
    const Vec2& p2_a,
    const Vec2& p1_b,
    const Vec2& p2_b,
    F32& distance,
    Vec2& normal
) {
    Vec2 A = p2_a - p1_a;   // plane direction
    Vec2 B = p2_b - p1_b;   // moving vertex direction

    Vec2 diff = p1_b - p1_a;

    F32 denom = vec2_cross(B, A);

    // paralell
    if (fabsf(denom) < 1e-8f) { return false; }

    F32 dist = vec2_cross(A, diff) / denom;
    F32 u = vec2_cross(B, diff) / denom;

    if (dist < 0.f || dist > 1.f) return false;
    if (u < 0.f || u > 1.f) return false;

    distance = dist;
    normal = { A.y, -A.x };
    normal = vec2_normal(normal);

    return true;
}