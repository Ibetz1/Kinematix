#ifndef _PSX_PARTITION_H
#define _PSX_PARTITION_H

#include "main.h"
#include "glx_shape.h"
#include "psx_spacial.h"
#include "psx_collider.h"
#include "psx_ray.h"

struct BvhNode {
    AABB box;
    Inst parent = NO_INSTANCE;
    Inst child1 = NO_INSTANCE;
    Inst child2 = NO_INSTANCE;
    Inst collider = NO_INSTANCE;
};

bool bvh_is_leaf(const BvhNode& node);

Inst bvh_new_node();

BvhNode& bvh_get_node(Inst id);

void bvh_set_node_parent(Inst id, Inst parent);

const AABB& bvh_get_node_box(Inst id);

U32 bvh_partition_ids(Inst* ids, U32 count, Axis axis, F32 split);

Inst bvh_build_recursive(Inst* ids, U32 count);

void bvh_build(Inst* colliders, U32 count);

void bvh_render_node(U32 node_id);

void bvh_render();

void bvh_calculate_manifolds();

struct PsxRay;
struct PsxRayResult;
PsxRayResult bvh_cast_ray(
    const PsxRay& ray,
    bool search_groups = false,
    bool search_layer = false
);

#endif