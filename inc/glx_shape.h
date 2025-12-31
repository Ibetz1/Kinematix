#ifndef _GLX_SHAPE_H
#define _GLX_SHAPE_H

#include "main.h"
#include "gl_wrapper.h"
#include "gl_express.h"
#include "gl_frame.h"
#include "glx_camera.h"
#include "glx_view.h"
#include "gl_express_shape.h"
#include "config.h"

enum Shape {
    SHAPE_NONE   = 0 << 0,
    SHAPE_CIRCLE = 1 << 0,
    SHAPE_RECT   = 1 << 1,
    SHAPE_POLY   = 1 << 2,
    SHAPE_POINT  = 1 << 3
};

// define a polygon as a static buffer

void shape_color(Color color);

void shape_draw_lines();

void shape_draw_fill();

void shape_draw_mode(GLXDrawMode mode);

GLXDrawMode shape_get_draw_mode();

Color shape_get_color();

void shape_polygon(Vec2* vertices, U32 count);

void shape_circle(Vec2 pos, F32 radius);

void shape_point(Vec2 pos);

void shape_bounding_box(AABB box);

void shape_line(Vec2 a, Vec2 b);

/*
    identities
*/

extern GlxPolygon shape_identity_square;
extern GlxPolygon shape_identity_triangle;
extern GlxPolygon shape_identity_pentagon;
extern GlxPolygon shape_identity_octogon;

#endif