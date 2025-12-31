#include "gl_express.h"

// Uniform helpers
S32 glx_uni(U32 prog, const char* name) {
    S32 loc = glGetUniformLocation(prog, name);
    if (loc < 0) THROW("Uniform not found: %s", name);
    return loc;
}

Status glx_set_float(S32 loc, F32 v) {
    glUniform1f(loc, v);
    return OK;
}

Status glx_set_vec2(S32 loc, Vec2 p) {
    glUniform2f(loc, p.x, p.y);
    return OK;
}

Status glx_set_vec2(S32 loc, F32 x, F32 y) {
    glUniform2f(loc, x, y);
    return OK;
}

Status glx_set_vec4(S32 loc, F32 r, F32 g, F32 b, F32 a) {
    glUniform4f(loc, r, g, b, a);

    return OK;
}

Status glx_set_mat4(S32 loc, const F32* m16_col_major) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, m16_col_major);

    return OK;
}

// Make a column-major ortho matrix mapping (0,0)-(w,h) to NDC, with Y down
void glx_make_ortho_px(F32 w, F32 h, F32* M) {
    const F32 l = 0.0f, r = w;
    const F32 t = 0.0f, b = h;   // top=0, bottom=h  (Y-down)

    // Standard ortho:
    // [ 2/(r-l)    0         0   -(r+l)/(r-l) ]
    // [ 0          2/(t-b)   0   -(t+b)/(t-b) ]
    // [ 0          0        -1    0           ]
    // [ 0          0         0    1           ]
    const F32 tx = -(r + l) / (r - l);
    const F32 ty = -(t + b) / (t - b);
    M[0]= 2.0f/(r-l); M[4]= 0.0f;       M[8]= 0.0f;  M[12]= tx;
    M[1]= 0.0f;       M[5]= 2.0f/(t-b); M[9]= 0.0f;  M[13]= ty;
    M[2]= 0.0f;       M[6]= 0.0f;       M[10]= -1.0f;M[14]= 0.0f;
    M[3]= 0.0f;       M[7]= 0.0f;       M[11]= 0.0f; M[15]= 1.0f;
}

U32 glx_compile(U32 type, const char* src) {
    U32 s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    S32 ok = 0; 
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);

    if (!ok) {
        char log[2048]; glGetShaderInfoLog(s, 2048, nullptr, log);
        THROW("Shader compile error: %s", log);
        glDeleteShader(s);
        return 0;
    }

    return s;
}

U32 glx_make_program(const char* vsSrc, const char* fsSrc) {
    U32 vs = glx_compile(GL_VERTEX_SHADER, vsSrc);

    if (!vs) {
        return 0;
    }

    U32 fs = glx_compile(GL_FRAGMENT_SHADER, fsSrc);
    if (!fs) { 
        glDeleteShader(vs); 
        return 0; 
    }

    U32 prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);

    S32 ok = 0; 
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);

    if (!ok) {
        char log[2048]; glGetProgramInfoLog(prog, 2048, nullptr, log);
        THROW("Program link error: %s", log);
        glDeleteProgram(prog);
        return 0;
    }

    LOGI("Program linked: %u", prog);
    return prog;
}

Status glx_init_program(GLApp& app, GLX& glx, const char* vertex_shader, const char* fragment_shader) {
    glx.program = glx_make_program(default_vertex_shader, default_fragment_shader);
    
    if (!glx.program) { 
        glapp_shutdown(app); 
        return ERROR;
    }

    // program uniforms
    glx.uProj     = glx_uni(glx.program, "uProj");
    glx.uColor    = glx_uni(glx.program, "uColor");
    glx.uCamera   = glx_uni(glx.program, "uCamera");
    glx.uViewPort = glx_uni(glx.program, "uViewPort");
    glx.uZoom     = glx_uni(glx.program, "uZoom");

    glx_set_projection_2d(app, glx);
    glx_reset(glx);

    return OK;
}

Status glx_set_projection_2d(GLApp& app, GLX& glx) {
    glx_make_ortho_px((F32) app.width, (F32) app.height, glx.projection);
    glUseProgram(glx.program);
    glx_set_mat4(glx.uProj, glx.projection);

    return OK;
}

Status glx_use(GLX& glx) {
    glUseProgram(glx.program);
    return OK;
}

/*
    GLX modifiers
*/

Status glx_set_color(GLX& glx, U8 r, U8 g, U8 b, U8 a) {
    glx.cColor = {r, g, b, a};
    glx_set_vec4(glx.uColor, (F32) r / (255.f), (F32) g / (255.f), (F32) b / (255.f), (F32) a / (255.f));
    return OK;
}

Status glx_set_line_width(GLX& glx, F32 line_width) {
    if (line_width < 1.f) line_width = 1.f;
    glx.line_width = line_width;
    glLineWidth(glx.line_width);
    return OK;
}

Status glx_set_draw_mode(GLX& glx, GLXDrawMode mode) {
    glx.draw_mode = mode;
    return OK;
}

Status glx_reset(GLX& glx) {
    Status ret = OK;

    // fix returns
    ret = glx_set_line_width(glx, 1.f);
    ret = glx_set_draw_mode(glx, DRAW_FILL);
    ret = glx_set_color(glx, 
        GLX_DEFAULT_DRAW_COLOR_R, 
        GLX_DEFAULT_DRAW_COLOR_G, 
        GLX_DEFAULT_DRAW_COLOR_B, 
        GLX_DEFAULT_DRAW_COLOR_A
    );

    return ret;
}