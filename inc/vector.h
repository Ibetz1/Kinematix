#ifndef _VECTOR_H
#define _VECTOR_H

#include "main.h"

union Vec2 {
    struct { F32 x, y; };
    struct { F32 w, h; };
    F32 raw[2];
};

union Vec4 {
    struct { F32 x, y, z, g; };
};

union Color {
    struct { U8 r, g, b, a; }; 
};

inline bool operator==(const Vec2& a, const Vec2& b) { return a.x == b.x && a.y == b.y; }
inline bool operator!=(const Vec2& a, const Vec2& b) { return a.x != b.x || a.y != b.y; }
inline bool operator< (const Vec2& a, const Vec2& b) { return (a.x < b.x) || (a.x == b.x && a.y < b.y);  }
inline bool operator> (const Vec2& a, const Vec2& b) { return (a.x > b.x) || (a.x == b.x && a.y > b.y);  }
inline bool operator<=(const Vec2& a, const Vec2& b) { return (a < b) || (a == b); }
inline bool operator>=(const Vec2& a, const Vec2& b) { return (a > b) || (a == b); }

inline Vec2 operator-(const Vec2& v) { return { -v.x, -v.y }; }
inline Vec2& operator+=(Vec2& a, const Vec2& b) { a.x += b.x; a.y += b.y; return a; }
inline Vec2& operator-=(Vec2& a, const Vec2& b) { a.x -= b.x; a.y -= b.y; return a; }
inline Vec2& operator*=(Vec2& a, const Vec2& b) { a.x *= a.x; a.y *= b.y; return a; }
inline Vec2& operator/=(Vec2& a, const Vec2& b) { a.x /= a.x; a.y /= b.y; return a; }

inline Vec2& operator+=(Vec2& a, F32 v) { a.x += v; a.y += v; return a; }
inline Vec2& operator-=(Vec2& a, F32 v) { a.x -= v; a.y -= v; return a; }
inline Vec2& operator*=(Vec2& a, F32 v) { a.x *= v; a.y *= v; return a; }
inline Vec2& operator/=(Vec2& a, F32 v) { a.x /= v; a.y /= v; return a; }

inline Vec2 operator+(const Vec2& a, F32 b) { return { a.x + b, a.y + b }; }
inline Vec2 operator-(const Vec2& a, F32 b) { return { a.x - b, a.y - b }; }
inline Vec2 operator*(const Vec2& a, F32 b) { return { a.x * b, a.y * b }; }
inline Vec2 operator/(const Vec2& a, F32 b) { return { a.x / b, a.y / b }; }

inline Vec2 operator+(Vec2 a, Vec2 b) { return { a.x + b.x, a.y + b.y }; }
inline Vec2 operator-(Vec2 a, Vec2 b) { return { a.x - b.x, a.y - b.y }; }
inline Vec2 operator*(Vec2 a, Vec2 b) { return { a.x * b.x, a.y * b.y }; }
inline Vec2 operator/(Vec2 a, Vec2 b) { return { a.x / b.x, a.y / b.y }; }

inline Vec2 vec2_perp(const Vec2& a) { return { -a.y, a.x }; }
inline F32 vec2_dot(const Vec2& a, const Vec2& b) { return a.x * b.x + a.y * b.y; }
inline F32 vec2_cross(const Vec2& a, const Vec2& b) { return a.x * b.y - b.x * a.y; }
inline Vec2 vec2_cross(F32 w, const Vec2& r) { return { -w * r.y, w * r.x }; }
inline F32 vec2_length(const Vec2& v) { return sqrtf(vec2_dot(v, v)); }
inline F32 vec2_length_sq(const Vec2& v) { return vec2_dot(v, v); }

inline F32 f32_clamp(F32 a, F32 mi, F32 ma) {
    return (a < mi) ? mi : (a > ma) ? ma : a;
}

inline Vec2 vec2_clamp(const Vec2& v, const Vec2& minv, const Vec2& maxv) {
    Vec2 r;
    r.x = v.x < minv.x ? minv.x : (v.x > maxv.x ? maxv.x : v.x);
    r.y = v.y < minv.y ? minv.y : (v.y > maxv.y ? maxv.y : v.y);
    return r;
}

inline Vec2 vec2_normal(const Vec2& v, Vec2 fallback = { 0, 0 }) { 
    const F32 len = vec2_length(v);
    if (len == 0.f) return fallback;
    return { v.x / len, v.y / len };
}

inline Vec2 vec2_normal(const Vec2& v, F32 len, Vec2 fallback = { 0, 0 }) { 
    if (len == 0.f) return fallback;
    return { v.x / len, v.y / len };
}

inline Vec2 vec2_rotate(const Vec2& v, F32 s, F32 c) {
    return {v.x * c - v.y * s, v.x * s + v.y * c};
}

inline Vec2 vec2_rotate(const Vec2& v, F32 a) {
    F32 s = sinf(a), c = cosf(a);
    return { v.x * c - v.y * s, v.x * s + v.y * c };
}

inline Vec2 vec2_rotate(const Vec2& v, const Vec2& o, F32 a) {
    Vec2 t = v - o;
    Vec2 r = vec2_rotate(t, a);
    return r + o;
}

inline Vec2 vec2_rotate(const Vec2& v, const Vec2& o, F32 s, F32 c) {
    Vec2 t = v - o;
    Vec2 r = vec2_rotate(t, s, c);
    return r + o;
}

inline F32 vec2_angle(const Vec2& v) {
    F32 a = atan2f(v.y, v.x);
    return (a < 0.f) ? (a + 2 * M_PI) : a;
}

inline Vec2 area_from_radius(const F32 radius) {
    return { 2 * radius, 2 * radius };
}

inline F32* vec2_as_array(Vec2& v) {
    return (F32*) &v;
}

inline F32* vec2_as_array(Vec2* v) {
    return (F32*) v;
}

#endif