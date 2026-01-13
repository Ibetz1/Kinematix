#include "psx_material.h"

static PsxMaterial g_materials[CFG_MAX_MATERIALS] = { };
static U32 g_materials_free[CFG_MAX_MATERIALS] = { };
static U32 g_materials_free_top = 0;
static U32 g_next_material = 0;

PsxMaterial g_default_material = {
    .friction = 0.f,
    .restitution = 0.f,

    .id = NO_INSTANCE,
    .in_use = true,
    .user_data = nullptr
};

PsxMaterial& material_get(Inst material) {
    if (material >= g_next_material) {
        THROW("Manifold: attempt to get invalid material");
    }

    return g_materials[material];
}

PsxMaterial& material_alloc() {
    Inst material;

    if (g_materials_free_top > 0) {
        g_materials_free_top--;
        material = g_materials_free[g_materials_free_top];
    }

    else {
        if (g_next_material >= CFG_MAX_MANIFOLDS) {
            THROW("Physics: no more materials");
        }

        material = g_next_material++;
    }


    // instance new material at index
    PsxMaterial& m = g_materials[material];

    if (m.in_use) {
        THROW("Physics: got material in use @ index=%i", material);
    }

    m.id = material;
    m.in_use = true;
    m.user_data = nullptr;

    return m;
}

void material_free(Inst material) {
    PsxMaterial& m = material_get(material);
        
    if (!m.in_use) {
        return;
    }

    m.in_use = false;
    m.user_data = nullptr;

    if (g_materials_free_top >= CFG_MAX_MANIFOLDS) {
        THROW("Physics: no more free slots");
    }

    g_materials_free[g_materials_free_top++] = m.id;
}

Inst material_new(PsxMaterialConfig config) {
    PsxMaterial& material = material_alloc();

    material.friction = config.friction;
    material.restitution = config.restitution;

    return material.id;
};

F32 material_get_friction(Inst material) {
    return (material == NO_INSTANCE) ? g_default_material.friction : material_get(material).friction;
}

F32 material_get_restitution(Inst material) {
    return (material == NO_INSTANCE) ? g_default_material.restitution : material_get(material).restitution;
}