#ifndef _PSX_MOTION
#define _PSX_MOTION

#include "main.h"
#include "vector.h"
#include "config.h"

enum PsxSpacialFlag : U32 {
    SPACIAL_FLAG_NONE    = 0 << 0,
    SPACIAL_FLAG_STATIC  = 1 << 0,
    SPACIAL_FLAG_RIGID   = 1 << 1,
    SPACIAL_FLAG_NO_GRAV = 1 << 2,
};

struct PsxSpacialConfig {
    Vec2 pos  = { 0, 0 };
    Vec2 ivel = { 0, 0 };
    Vec2 iforce = { 0, 0 };

    F32 iang     = 0.f;
    F32 iang_vel = 0.f;
    F32 iang_acc = 0.f;

    F32 mass = 1.f;
    F32 rot_factor = 1.f;

    U32 flags = SPACIAL_FLAG_NONE;
    U32 layer = 0;
    U32 group = 0;

    void* user_data = nullptr;
};

struct PsxSpacial {
    void* user_data;

    // positional
    Vec2 vel;
    Vec2 force;
    Vec2 pos;
    Vec2 prev_pos;

    // angular
    F32 ang_vel;
    F32 torque;
    F32 ang;
    F32 prev_ang;

    // properties
    F32 mass;
    F32 inv_mass;
    F32 inertia;
    F32 inv_inertia;

    // data
    U32 flags;
    U32 index;
    U32 layer; // collision layer
    U32 group; // category bit (player, enemy, world, etc)

    bool in_use;
};

// void spacial_update(F32 dt);

void spacial_integrate_velocities(F32 dt);
void spacial_integrate_positions(F32 dt);

PsxSpacial& spacial_get(Inst spacial);

PsxSpacial& spacial_alloc();

void spacial_free(Inst spacial);

Inst spacial_new(PsxSpacialConfig cfg);

void spacial_move_to(Inst spacial, Vec2 pos);
void spacial_move_to(PsxSpacial& spacial, Vec2 pos);

void spacial_add_force(Inst spacial, Vec2 impulse);
void spacial_accellarate(PsxSpacial& spacial, Vec2 impulse);

void spacial_impulse(Inst spacial, Vec2 impulse);
void spacial_impulse(PsxSpacial& spacial, Vec2 impulse);
void spacial_impulse(PsxSpacial& s, Vec2 impulse, Vec2 contact_point_world);

/*
    getters
*/
Vec2 spacial_get_pos(Inst spacial);

Vec2 spacial_get_prev_pos(Inst spacial);

Vec2 spacial_get_vel(Inst spacial);

Vec2 spacial_get_acc(Inst spacial);

F32 spacial_get_ang(Inst spacial);

void spacial_render();

#endif