#ifndef _GL_WRAPPER_H
#define _GL_WRAPPER_H

#include "main.h"

// Basic SDL + OpenGL wrapper
struct GLApp {
    SDL_Event event;

    SDL_Window*   window   = nullptr;
    SDL_GLContext context  = nullptr;
    int width    = 0;
    int height   = 0;

    std::function<void(SDL_Event)> SDL_event_hook = [](SDL_Event){};
};

// Initializes SDL, creates a window, OpenGL context, and loads GL functions.
Status glapp_init(GLApp& app, const char* title, S32 width, S32 height,
                  S32 glMajor = 3, S32 glMinor = 3,
                  bool coreProfile = true, bool vsync = false);

// Poll events; returns false on quit
bool glapp_poll(GLApp& app);

// Begin rendering frame
inline void glapp_begin_frame(U8 r = 0, U8 g = 0, U8 b = 0, U8 a = 0) {
    glClearColor((F32) r / 255.f, (F32) g / 255.f, (F32) b / 255.f, (F32) a / 255.f);
    glClear(GL_COLOR_BUFFER_BIT);
}

// End frame
inline void glapp_end_frame(GLApp& app) {
    SDL_GL_SwapWindow(app.window);
}

// Shutdown and cleanup
void glapp_shutdown(GLApp& app);

void glapp_bind_sdl_events(GLApp& app, std::function<void(SDL_Event)> hook);

#endif
