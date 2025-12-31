#ifndef _GLX_VIEW_H
#define _GLX_VIEW_H

#include "main.h"
#include "gl_wrapper.h"
#include "gl_express.h"
#include "gl_frame.h"
#include "glx_camera.h"

enum ViewCullingState {
    CULL_NONE,
    CULL_LAZY,
    CULL_FULL
};

struct View {
    static GLApp* app;
    static GLX glx;
    static Frame frame;
    static Camera camera;
};

void view_init(GLApp& app);

void view_update(F32 dt);

void view_start();

void view_finish();

void view_move_camera(Vec2 pos, Vec2 area = { 0 });

void view_step_zoom(F32 dir);

#endif