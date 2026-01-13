#ifndef _PSX_MATERIAL_H
#define _PSX_MATERIAL_H

#include "main.h"
#include "config.h"

struct PsxMaterialConfig {
    F32 friction = 0.f;
    F32 restitution = 0.f;
};

struct PsxMaterial {
    F32 friction;
    F32 restitution;

    U32 id;
    bool in_use;
    void* user_data;
};

// default material
extern PsxMaterial g_default_material;

PsxMaterial& material_get(Inst material);

PsxMaterial& material_alloc();

void material_free(Inst material);

Inst material_new(PsxMaterialConfig config);

F32 material_get_friction(Inst material);

F32 material_get_restitution(Inst material);
#endif 