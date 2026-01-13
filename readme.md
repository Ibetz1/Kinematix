Kinematix - A Box2D Clone

Build Project
```
g++ src/*.cpp src/libs/glad/*.c -Iinc -Isrc/libs -I. -lSDL2 -lopengl32
```

Required loop functions
```c++
void phy_step(F32 dt) {

    // update all motion properties
    spacial_integrate_velocities(dt);

    // update all world positions
    spacial_integrate_positions(dt);

    // filter usable colliders
    collider_filter_updated();

    // build a fresh BVH from colliders
    collider_build_bvh();

    // generate collision manifolds
    bvh_calculate_manifolds();

    // solve active collisions
    manifolds_solve(dt);

}
```

#### Spacials
Spacials hold all possition and velocity data along with physical properties.
```c++
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

Inst spacial = spacial_new(PsxSpacialConfig); // returns reference to spacial
```

Spacial helper functions
```c++
Vec2 spacial_get_pos(Inst);
Vec2 spacial_get_prev_pos(Inst);
Vec2 spacial_get_vel(Inst);
Vec2 spacial_get_acc(Inst);
F32 spacial_get_ang(Inst);

// set spacial pos directly
void spacial_move_to(PsxSpacial& s, Vec2 pos); 
void spacial_move_to(Inst spacial, Vec2 pos);

// apply force to be integrated
void spacial_accellarate(PsxSpacial& s, Vec2 force);

// impulse applied directly to velocity
void spacial_impulse(PsxSpacial& s, Vec2 impulse); 
void spacial_impulse(PsxSpacial& s, Vec2 impulse, Vec2 contact_point_world); // adds inertia
```

#### Colliders
Colliders can be attached to spacials at an offset to interract with the world
```

```