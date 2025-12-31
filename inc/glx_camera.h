#ifndef _GLX_CAMERA_H
#define _GLX_CAMERA_H

#include "main.h"
#include "vector.h"
#include "config.h"

struct Camera {
    Vec2 targ_pos;
    Vec2 cur_pos;
    Vec2 pos_speed;
    F32 cur_zoom;
    F32 targ_zoom;
    F32 zoom_speed;
};

void camera_init(Camera& camera);

void camera_update(Camera& camera, F32 dt);

void camera_move_to(Camera& camera, Vec2 pos);

void camera_set_zoom(Camera& camera, F32 goal);

void camera_step_zoom(Camera& camera, F32 dir);

#endif