#ifndef _PSX_COLLIDER_H
#define _PSX_COLLIDER_H

#include "main.h"
#include "vector.h"
#include "config.h"
#include "psx_spacial.h"
#include "glx_shape.h"
#include "psx_manifold.h"
#include "psx_partition.h"

enum PsxColliderPhase : U32 {
    COLLIDER_PHASE_NONE   = 0 << 0,  // not running any collision checks
    COLLIDER_PHASE_BROAD  = 1 << 0,  // bounding boxes returned true, check collision shape
    COLLIDER_PHASE_NARROW = 1 << 1,  // running direct collision checks
    COLLIDER_PHASE_RESOLVE = 1 << 2  // running collision resolution (has manifold)
};

struct PsxCircleCollider {
    F32 radius;
};

struct PsxPolyCollider {
    Vec2* identity;
    Vec2* transform;
    Vec2 center;
    F32 scale;
    U32 count;
};

struct PsxColliderConfig {
    Inst spacial = NO_INSTANCE;
    Inst material = NO_INSTANCE;
    Vec2 offset;
    void* user_data;
};

struct PsxCollider {
    void* user_data;

    /*
        basic info
    */
    AABB bounding_box;

    union {
       PsxCircleCollider circ;
       PsxPolyCollider   poly;
    };
    
    Shape shape;
    Vec2 offset;

    Inst spacial;       // reference to spacial object
    U32 id;             // index of collider in g_colliders
    U32 phase;          // current phase of collision

    U8* heap_buffer;
    U32 alloc_bytes = 0;
};

PsxCollider& collider_get(U32 index); // get reference to existing collider

PsxCollider& collider_alloc();

void collider_free(U32 index);

void collider_update(Inst collider);

void collider_filter_updated();

void collider_build_bvh();

void collider_make_heap_buffer(PsxCollider& collider, U32 size);


/*
    allocate new shapes
*/

Inst collider_new_circle(F32 radius, PsxColliderConfig cfg = {});

Inst collider_new_rect(Vec2 area, PsxColliderConfig cfg = {});

Inst collider_new_poly(GlxPolygon identity, F32 scale = 1.f, PsxColliderConfig cfg = {});

/*
    get properties
*/
Vec2 collider_get_pos(const PsxCollider& c);

U32 collider_get_group(const PsxCollider& c);

U32 collider_get_layer(const PsxCollider& c);

U32 collider_get_flags(const PsxCollider& c);

U32 collider_get_flags(const PsxCollider& c, U32 flag);

bool collider_compare_groups(const PsxCollider& a, const PsxCollider& b);

bool collider_compare_layer(const PsxCollider& a, const PsxCollider& b);

Vec2 collider_get_pos(Inst collider);

F32 collider_get_radius(Inst collider);

const AABB& collider_get_bounding_box(Inst collider);

/*
    base utility
*/
void collider_draw(Inst collider);

void collider_draw_all();

U32 count_colliders();

#endif