#ifndef _PSX_MANIFOLD_H
#define _PSX_MANIFOLD_H

#include "main.h"
#include "vector.h"
#include "config.h"
#include "psx_collider.h"
#include "psx_material.h"
#include "psx_algo.h"

#define MM_MAX_CONTACT_PTS 2

struct PsxManifold {
    void* user_data;

    Vec2 normal;
    Vec2 tangent;
    Vec2 contact;

    Inst collider_a;
    Inst collider_b;

    U32 index;
    F32 depth;

    bool colliding;
    bool in_use;
};

PsxManifold& manifold_get(Inst manifold); // get reference to existing manifold

PsxManifold& manifold_alloc();

void manifold_free(Inst manifold);

Inst manifold_new(
    Inst collider_a, 
    Inst collider_b, 
    bool colliding,
    Vec2 normal  = { 0, 0 },
    Vec2 tangent = { 0, 0 },
    Vec2 contact = { 0 },
    F32  depth         = 0.f
);

void manifold_solve(const PsxManifold& m, F32 dt);

void manifolds_solve(F32 dt);

bool manifold_get_colliding(Inst manifold);

void manifolds_render();

/*
    get a manifolder between two colliders
*/

Inst manifold_generate(U32 collider_a, U32 collider_b);

U32 count_manifolds();

#endif