#include "glx_view.h"

GLApp* View::app = nullptr;
GLX View::glx = { 0 };
Frame View::frame = { 0 };
Camera View::camera = { 0 };

void view_init(GLApp& app) {
    glx_init_program(app, View::glx, default_vertex_shader, default_fragment_shader);
    frame_init(View::frame);

    View::app = &app;

    glx_set_vec2(View::glx.uViewPort, app.width, app.height);

    camera_init(View::camera);
}

void view_update(F32 dt) {
    camera_update(View::camera, dt);
    glx_set_vec2(View::glx.uCamera, View::camera.cur_pos);
    glx_set_float(View::glx.uZoom, View::camera.cur_zoom);
}

void view_start() {
    glapp_begin_frame();
    frame_start(View::frame);
    glx_use(View::glx);
}

void view_finish() {
    glx_reset(View::glx);
    frame_stop();
    glapp_end_frame(*View::app);
}

void view_move_camera(Vec2 pos, Vec2 area) {
    Vec2 screen = {(F32) View::app->width, (F32) View::app->height};
    camera_move_to(View::camera, pos - (screen - area) * 0.5f);
}

void view_step_zoom(F32 dir) {
    camera_step_zoom(View::camera, dir);
}