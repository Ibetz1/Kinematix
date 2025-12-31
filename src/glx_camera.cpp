#include "glx_camera.h"

void camera_init(Camera& camera) {
    camera.targ_pos = { 0 };
    camera.cur_pos = { 0 };
    camera.cur_zoom = 1.f;
    camera.targ_zoom = 1.f;
    camera.pos_speed = { 0.9f, 0.9f };
    camera.zoom_speed = 0.9f;
}

void camera_update(Camera& camera, F32 dt) {
    vec2_clamp(camera.pos_speed, {0, 0}, {1, 1});
    Vec2 pos_alpha = { 1.f - powf(1.f - camera.pos_speed.x, dt), 1.f - powf(1.f - camera.pos_speed.y, dt) };
    camera.cur_pos += (camera.targ_pos - camera.cur_pos) * pos_alpha;

    camera.zoom_speed = f32_clamp(camera.zoom_speed, 0.f, 1.f);
    camera.cur_zoom = f32_clamp(camera.cur_zoom, CFG_MIN_ZOOM, CFG_MAX_ZOOM);

    F32 zoom_alpha = 1.f - powf(1.f - camera.zoom_speed, dt);
    camera.cur_zoom += (camera.targ_zoom - camera.cur_zoom) * zoom_alpha;
};

void camera_move_to(Camera& camera, Vec2 pos) {
    camera.targ_pos = pos;
};

void camera_set_zoom(Camera& camera, F32 goal) {
    camera.targ_zoom = goal;

    f32_clamp(camera.targ_zoom, CFG_MIN_ZOOM, CFG_MAX_ZOOM);
}

void camera_step_zoom(Camera& camera, F32 dir) {
    camera_set_zoom(camera, camera.cur_zoom + camera.zoom_speed * dir);
}