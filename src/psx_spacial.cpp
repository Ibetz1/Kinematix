#include "psx_spacial.h"
#include "glx_shape.h"

static PsxSpacial g_spacials[CFG_MAX_SPACIALS] = { };
static U32 g_spacials_free[CFG_MAX_SPACIALS] = { };
static U32 g_spacials_free_top = 0;
static U32 g_next_spacial = 0;

// global properties
static F32 gravity = 1000.f;

PsxSpacial& spacial_get(Inst spacial) {
    if (spacial >= g_next_spacial) {
        THROW("Collider: attempt to get invalid s");
    }

    return g_spacials[spacial];
}

PsxSpacial& spacial_alloc() {
    Inst spacial;

    if (g_spacials_free_top > 0) {
        g_spacials_free_top--;
        spacial = g_spacials_free[g_spacials_free_top];
    }

    else {
        if (g_next_spacial >= CFG_MAX_SPACIALS) {
            THROW("Physics: no more colliders");
        }

        spacial = g_next_spacial++;
    }


    // instance new collider atspacial
    PsxSpacial& s = g_spacials[spacial];

    if (s.in_use) {
        THROW("Physics: got s in use @spacial=%i",spacial);
    }

    s.index =spacial;
    s.in_use = true;
    s.user_data = nullptr;

    return s;
}

void spacial_free(Inst spacial) {
    PsxSpacial& s = spacial_get(spacial);
        
    if (!s.in_use) {
        return;
    }

    s.in_use = false;
    s.user_data = nullptr;

    if (g_spacials_free_top >= CFG_MAX_COLLIDERS) {
        THROW("Physics: no more free slots for spacials");
    }

    g_spacials_free[g_spacials_free_top++] = s.index;
}

U32 spacial_new(PsxSpacialConfig cfg) {
    PsxSpacial& s = spacial_alloc();

    s.pos = cfg.pos;
    s.prev_pos = s.pos;
    s.vel = cfg.ivel;
    s.force = cfg.iforce;

    s.ang = cfg.iang;
    s.prev_ang = s.ang;
    s.ang_vel = cfg.iang_vel;
    s.torque = cfg.iang_acc;

    if (cfg.mass > 0.f && !(cfg.flags & SPACIAL_FLAG_STATIC)) {
        s.mass = cfg.mass;
        s.inv_mass = 1.f / s.mass;

        s.inertia = cfg.mass * cfg.rot_factor * CFG_INTERTIA_SCALAR;
        s.inv_inertia = 1.f / s.inertia;
    } else {
        cfg.flags |= SPACIAL_FLAG_STATIC; // flag static if mass is weird
        s.mass = 0.f;
        s.inv_mass = 0.f;
        s.inertia = 0.f;
        s.inv_inertia = 0.f;
    }

    s.flags = cfg.flags;
    s.user_data = cfg.user_data;
    s.group = cfg.group;
    s.layer = cfg.layer;
    
    return s.index;
}

void spacial_move_to(PsxSpacial& s, Vec2 pos) {
    s.pos = pos;
}

void spacial_move_to(Inst spacial, Vec2 pos) {
    spacial_move_to(spacial_get(spacial), pos);
}

void spacial_accellarate(PsxSpacial& s, Vec2 force) {
    if (s.flags & SPACIAL_FLAG_STATIC) return;

    s.force += force;
}

void spacial_impulse(PsxSpacial& s, Vec2 impulse) {
    if (s.flags & SPACIAL_FLAG_STATIC) return;

    s.vel += impulse * s.inv_mass;
}


void spacial_impulse(PsxSpacial& s, Vec2 impulse, Vec2 contact_point_world) {
    if (s.flags & SPACIAL_FLAG_STATIC) return;
    if (s.inv_mass == 0.f && s.inv_inertia == 0.f) return;

    // linear part
    if (s.inv_mass > 0.f) {
        s.vel += impulse * s.inv_mass;
    }

    if (!(s.flags & SPACIAL_FLAG_RIGID)) return; // dont do rigid body stuff

    // angular part
    if (s.inv_inertia > 0.f) {
        Vec2 r = contact_point_world - s.pos;  // COM -> contact
        F32 tau = vec2_cross(r, impulse);
        s.ang_vel += tau * s.inv_inertia;
    }
}

void spacial_integrate_velocities(F32 dt) {
    for (int i = 0; i < g_next_spacial; ++i) {
        PsxSpacial& s = spacial_get(i);
        if (!s.in_use) continue;

        if (s.flags & SPACIAL_FLAG_STATIC) continue;

        if (s.inv_mass == 0.f) {
            // clear forces but do not integrate
            s.force   = {0, 0};
            s.torque  = 0.f;
            continue;
        }

        // Gravity
        Vec2 g_vector = {0, 0};
        if (!(s.flags & SPACIAL_FLAG_NO_GRAV)) {
            g_vector.y = gravity;
        }

        // Linear acceleration
        Vec2 acc = (s.force * s.inv_mass) + g_vector;

        // Update linear velocity
        s.vel += acc * dt;
        s.vel *= expf(-CFG_DRAG_COEFFICIENT * dt);

        // Angular acceleration
        s.ang_vel += s.torque * dt;
        s.ang_vel *= expf(-CFG_ANG_DRAG_COEFFICIENT * dt);

        // Clear accumulated forces/torques
        s.force  = {0, 0};
        s.torque = 0.f;
    }
}

void spacial_integrate_positions(F32 dt) {
    for (int i = 0; i < g_next_spacial; ++i) {
        PsxSpacial& s = spacial_get(i);
        if (!s.in_use) continue;

        if (s.flags & SPACIAL_FLAG_STATIC) continue;

        if (s.inv_mass == 0.f) continue;

        // Save previous position before integrating
        s.prev_pos = s.pos;
        s.prev_ang = s.ang;

        // Position update
        s.pos += s.vel * dt;

        // Angle update
        s.ang += s.ang_vel * dt;
    }
}

Vec2 spacial_get_pos(Inst spacial) {
    return spacial_get(spacial).pos;
}

Vec2 spacial_get_prev_pos(Inst spacial) {
    return spacial_get(spacial).prev_pos;
}

Vec2 spacial_get_vel(Inst spacial) {
    return spacial_get(spacial).vel;
}

Vec2 spacial_get_acc(Inst spacial) {
    return spacial_get(spacial).force;
}

F32 spacial_get_ang(Inst spacial) {
    return spacial_get(spacial).ang;
}

void spacial_render() {

    #if CFG_RENDER_FORCES

    for (int i = 0; i < g_next_spacial; ++i) {
        const PsxSpacial& spacial = spacial_get(i);
        if (!spacial.in_use) continue;

        shape_line(spacial.pos, spacial.pos + spacial.vel / 50.f);

        // printf("%.4f %.4f %.4f %.4f\n", spacial.pos.x, spacial.pos.y, spacial.vel.x, spacial.vel.y);
    }

    #endif

}


// ew
void spacial_add_force(Inst spacial, Vec2 impulse) 
{ spacial_accellarate(spacial_get(spacial), impulse); }
void spacial_impulse(Inst spacial, Vec2 impulse)     
{ spacial_impulse    (spacial_get(spacial), impulse); }