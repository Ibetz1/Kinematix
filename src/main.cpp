#include "gl_express_shape.h"
#include "gl_frame.h"
#include "glx_view.h"
#include "glx_shape.h"
#include "psx_collider.h"
#include "psx_manifold.h"

void physics_step(F32 dt) {

    spacial_integrate_velocities(dt);
    spacial_integrate_positions(dt);
    collider_filter_updated();
    collider_build_bvh();
    bvh_calculate_manifolds();
    manifolds_solve(dt);

}

int main() {
    GLApp app;
    if (glapp_init(app, "SDL + OpenGL", 800, 600)) {
        THROW("Failed to initialize application");
        return ERROR;
    }

    view_init(app);

    Uint32 last_ticks = SDL_GetTicks();
    F32 dt = 1.f;

    Inst player = spacial_new({
        .pos = {0, 0},
        .flags = SPACIAL_FLAG_RIGID,
        .layer = 0,
    });
        collider_new_rect({32, 50}, 
            {
                .spacial = player 
            }
        );

    Inst world_spacial = spacial_new({
        .pos = {0, 500},
        .flags = SPACIAL_FLAG_STATIC,
        .group = 1
    });

    Inst world_material = material_new({
        .friction = 0.f,
        .restitution = 1.f,
    });

    collider_new_rect({1000, 100}, 
        { 
            .spacial = world_spacial,
            .material = world_material,
        }
    );

    collider_new_rect({100, 1000}, 
        { 
            .spacial = world_spacial, 
            .material = world_material,
            .offset = { 500, -500},
        }
    );

    collider_new_rect({100, 1000}, 
        { 
            .spacial = world_spacial, 
            .material = world_material,
            .offset = {-500, -500},
        }
    );

    Inst loc = spacial_new({
        .pos = {0, 0},
        .flags = SPACIAL_FLAG_STATIC,
        .group = 1
    });
    
    Inst rect = collider_new_rect({50, 50}, { .spacial = loc});

    // hook to SDL events
    glapp_bind_sdl_events(app, [&](SDL_Event e) {
        if (e.type == SDL_MOUSEWHEEL) {
            if (e.wheel.y > 0) view_step_zoom(0.5);
            if (e.wheel.y < 0) view_step_zoom(-0.5);
        }
    });

    F32 seconds = 0;
    F32 q_seconds = 0.f;
    int frames = 0;

    while (glapp_poll(app)) {
        Uint32 now = SDL_GetTicks();
        dt = (now - last_ticks) / 1000.0f; // seconds
        last_ticks = now;

        seconds += dt;
        q_seconds += dt;
        frames++;

        if (seconds > 1) {
            printf("FPS=%i COLLIDERS=%i\n", frames, count_colliders());
            frames = 0;
            seconds = 0;
        }

        /*
            update pass
        */
        view_update(dt);
        shape_draw_lines();
        physics_step(dt);

        int mou_x, mou_y;
        const U8* kb_state = SDL_GetKeyboardState(NULL);
        const U32 mou_state = SDL_GetMouseState(&mou_x, &mou_y);
        constexpr float speed = 500.f;

        Vec2 impulse = { 0 };
        if (kb_state[SDL_SCANCODE_W]) { impulse.y -= 1.f; }
        if (kb_state[SDL_SCANCODE_S]) { impulse.y += 1.f; }
        if (kb_state[SDL_SCANCODE_A]) { impulse.x -= 1.f; }
        if (kb_state[SDL_SCANCODE_D]) { impulse.x += 1.f; }

        impulse = vec2_normal(impulse);
        impulse *= speed;

        spacial_add_force(player, impulse);

        view_move_camera(spacial_get_pos(player));

        PsxRay ray = {
            .origin = spacial_get_pos(player),
            .dir = vec2_normal({0, 1}),
            .max_dist = 30.f,
            .group = 1
        };

        F32 dist = ray.max_dist;

        PsxRayResult hit = ray_cast(ray);
        if (hit.touched) {
            dist = hit.dist;
        }

        if (kb_state[SDL_SCANCODE_SPACE] && hit.touched) { spacial_impulse(player, {0, -20000 * dt}); }

        /*
            render pass
        */
        view_start();
        shape_line(ray.origin, ray.origin + ray.dir * dist);

        collider_draw_all();
        bvh_render();
        manifolds_render();
        spacial_render();
        
        view_finish();
    }

    glapp_shutdown(app);
    return 0;
}