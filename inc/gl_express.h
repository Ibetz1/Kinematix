#ifndef _GL_EXPRESS_H
#define _GL_EXPRESS_H

/*
    dependencies
*/

#include "main.h"
#include "gl_wrapper.h"
#include "vector.h"

#define GLX_DEFAULT_LINE_WIDTH 1.f
#define GLX_DEFAULT_DRAW_MODE DRAW_FILL
#define GLX_DEFAULT_DRAW_COLOR_R 255
#define GLX_DEFAULT_DRAW_COLOR_G 255
#define GLX_DEFAULT_DRAW_COLOR_B 255
#define GLX_DEFAULT_DRAW_COLOR_A 255

// changeme
constexpr char default_vertex_shader[] = "\
// vertex shader (pos -> NDC via uProj)\n\
#version 330 core\n\
layout(location=0) in vec2 aPos;\n\
uniform mat4 uProj;\n\
uniform vec2 uCamera;\n\
uniform vec2 uViewPort;\n\
uniform float uZoom;\n\
\
void main() {\n\
    vec2 center = uViewPort * 0.5;\n\
    vec2 worldPos = (aPos - uCamera - center) * uZoom + center;\n\
    gl_Position = (uProj * vec4(worldPos, 0.0, 1.0));\n\
}\n\
";

constexpr char default_fragment_shader[] = "\
#version 330 core\n\
out vec4 FragColor;\n\
uniform vec4 uColor;\n\
void main() { FragColor = uColor; }\n\
";

enum GLXDrawMode : U32 {
    DRAW_FILL = 0x00,
    DRAW_LINES = 0x01,
};

struct GLX {

    // GLX program
    U32 program = { 0 };

    // static uniforms
    S32 uProj = { 0 };   // projection shader uniform
    S32 uColor = { 0 };  // color uniform
    S32 uCamera = { 0 }; // camera uniform
    S32 uZoom = { 0 }; // zoom amount
    S32 uViewPort = { 0 }; // screen specs

    Color cColor = { 255, 255, 255, 255 }; // current color

    // 2D orthographic projection
    F32 projection[16] = { 0 };

    // draw settings
    GLXDrawMode draw_mode = GLX_DEFAULT_DRAW_MODE;
    F32 line_width = GLX_DEFAULT_LINE_WIDTH;
};

// Uniform helpers
S32 glx_uni(U32 prog, const char* name);

Status glx_set_float(S32 loc, F32 v);

Status glx_set_vec2(S32 loc, Vec2 p);
Status glx_set_vec2(S32 loc, F32 x, F32 y);

Status glx_set_vec4(S32 loc, F32 r,F32 g,F32 b,F32 a);

Status glx_set_mat4(S32 loc, const F32* m16_col_major);

// Make a column-major ortho matrix mapping (0,0)-(w,h) to NDC, with Y down
void glx_make_ortho_px(F32 w, F32 h, F32* M);

// compile glx buffer
U32 glx_compile(U32 type, const char* src);

// make glx program
U32 glx_make_program(const char* vsSrc, const char* fsSrc);

Status glx_init_program(GLApp& app, GLX& glx, const char* vertex_shader, const char* fragment_shader);

Status glx_set_projection_2d(GLApp& app, GLX& glx);

Status glx_use(GLX& glx);

/*
    glx modifiers
*/

Status glx_set_color(GLX& glx, U8 r, U8 g, U8 b, U8 a);

Status glx_set_line_width(GLX& glx, F32 line_width);

Status glx_set_draw_mode(GLX& glx, GLXDrawMode mode);

Status glx_reset(GLX& glx);

#endif