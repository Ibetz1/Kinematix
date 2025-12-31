#ifndef _PSX_MATERIAL_H
#define _PSX_MATERIAL_H

#include "main.h"
#include "config.h"

struct PsxMaterialConfig {

};

struct PsxMaterial {
    
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

#endif 