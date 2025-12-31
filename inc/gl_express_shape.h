#ifndef _GL_EXPRESS_SHAPE_H
#define _GL_EXPRESS_SHAPE_H

#include "main.h"
#include "vector.h"
#include "gl_wrapper.h"
#include "gl_express.h"

typedef StaticBuffer<Vec2> GlxPolygon;
typedef struct { Vec2 min, max; } GlxBoundingBox, AABB;

bool glx_aabb_check(const AABB& a, const AABB& b);

AABB glx_aabb_merge(const AABB& a, const AABB& b);

Vec2 glx_aabb_center(const AABB& bb);

F32 glx_aabb_perimeter(const AABB& a);

/*
    glx shapes
*/

// draw generic polygon from vertex array
Status glx_shape_2d(GLX& glx, const F32* xy, S32 count);

// will rotate any polygon about an axis
void glx_rotate_poly_2d(const F32* in_verts, F32* out_verts, U32 vert_count, F32 cx, F32 cy, F32 angle);

// draw a rectangle
Status glx_rect_2d(GLX& glx, F32 x, F32 y, F32 w, F32 h, F32 angle = 0.f);

// draw a circle
Status glx_circle_2d(GLX& glx, F32 x, F32 y, F32 r, S32 lod = -1);

// transform polygon with angle, viewport, scale, position
void glx_transform_poly_2d(
    Vec2 pos, 
    Vec2* out, 
    const Vec2* identity, 
    U32 count, 
    F32 scale = 1.f, 
    F32 angle = 0.f, 
    GlxBoundingBox* bounding_box = nullptr,
    Vec2* center = nullptr
);

#endif