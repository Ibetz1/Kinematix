#include "glx_shape.h"

void shape_color(Color color) {
    glx_set_color(View::glx, color.r, color.g, color.b, color.a);
}

void shape_draw_lines() { 
    glx_set_draw_mode(View::glx, DRAW_LINES);
}

void shape_draw_fill() { 
    glx_set_draw_mode(View::glx, DRAW_FILL);
}

void shape_draw_mode(GLXDrawMode mode) {
    glx_set_draw_mode(View::glx, mode);
}

GLXDrawMode shape_get_draw_mode() {
    return View::glx.draw_mode;
}

Color shape_get_color() {
    return View::glx.cColor;
}

void shape_polygon(Vec2* vertices, U32 count) {
    glx_shape_2d(View::glx, (F32*) vertices, count);
}

void shape_circle(Vec2 pos, F32 radius) {
    glx_circle_2d(View::glx, pos.x, pos.y, radius);
}

void shape_point(Vec2 pos) {
    shape_circle(pos, CFG_POINT_RADIUS);
}

void shape_bounding_box(AABB box) {
    GlxPolygon identity = GlxPolygon({
        {box.min.x, box.min.y},
        {box.max.x, box.min.y},
        {box.max.x, box.max.y},
        {box.min.x, box.max.y},
    });

    glx_shape_2d(View::glx, (F32*) identity.data, identity.count);
}

void shape_line(Vec2 a, Vec2 b) {
    Vec2 verts[2] = { a, b };
    glx_shape_2d(View::glx, (F32*) verts, 2);
}

/*
    identities
*/

constexpr Vec2 sqr_identity[] = {{-1, -1},{-1, 1},{ 1, 1},{ 1, -1}};
constexpr Vec2 tri_identity[] = {{ 0, -0},{-1, 1},{ 1, 1}};
constexpr Vec2 pent_identity[] = {
    {  0.0f,   1.0f },
    { -0.9510565f,  0.3090170f },
    { -0.5877853f, -0.8090170f },
    {  0.5877853f, -0.8090170f },
    {  0.9510565f,  0.3090170f },
};
constexpr Vec2 oct_identity[] = {
    {  0.0f,   1.0f },
    { -0.7071068f,  0.7071068f },
    { -1.0f,   0.0f },
    { -0.7071068f, -0.7071068f },
    {  0.0f,  -1.0f },
    {  0.7071068f, -0.7071068f },
    {  1.0f,   0.0f },
    {  0.7071068f,  0.7071068f },
};

GlxPolygon shape_identity_square   = GlxPolygon(sqr_identity);
GlxPolygon shape_identity_triangle = GlxPolygon(tri_identity);
GlxPolygon shape_identity_pentagon = GlxPolygon(pent_identity);
GlxPolygon shape_identity_octogon  = GlxPolygon(oct_identity);