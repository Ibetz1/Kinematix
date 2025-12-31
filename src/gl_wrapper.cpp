#include "gl_wrapper.h"

Status glapp_init(GLApp& app, const char* title, int width, int height,
                int glMajor, int glMinor,
                bool coreProfile, bool vsync)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        THROW("SDL_Init failed: %s", SDL_GetError());
        return ERROR;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, glMajor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, glMinor);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    if (coreProfile)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    app.window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (!app.window) {
        THROW("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return ERROR;
    }

    app.context = SDL_GL_CreateContext(app.window);
    if (!app.context) {
        THROW("SDL_GL_CreateContext failed: %s", SDL_GetError());
        SDL_DestroyWindow(app.window);
        SDL_Quit();
        return ERROR;
    }

    if (SDL_GL_MakeCurrent(app.window, app.context) != 0) {
        THROW("SDL_GL_MakeCurrent failed: %s", SDL_GetError());
        SDL_GL_DeleteContext(app.context);
        SDL_DestroyWindow(app.window);
        SDL_Quit();
        return ERROR;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        THROW("Failed to initialize GLAD");
        SDL_GL_DeleteContext(app.context);
        SDL_DestroyWindow(app.window);
        SDL_Quit();
        return ERROR;
    }

    if (vsync)
        SDL_GL_SetSwapInterval(1);
    else
        SDL_GL_SetSwapInterval(0);

    app.width  = width;
    app.height = height;

    LOGI("OpenGL initialized (v%s)", glGetString(GL_VERSION));
    return OK;
}

bool glapp_poll(GLApp& app) {
    
    while (SDL_PollEvent(&app.event)) {
        if (app.event.type == SDL_QUIT)
            return false;
        if (app.event.type == SDL_KEYDOWN && app.event.key.keysym.sym == SDLK_ESCAPE)
            return false;

        app.SDL_event_hook(app.event);
    }
    return true;
}

void glapp_shutdown(GLApp& app) {
    if (app.context) SDL_GL_DeleteContext(app.context);
    if (app.window)  SDL_DestroyWindow(app.window);
    SDL_Quit();
    LOGI("OpenGL context destroyed, SDL shutdown complete");
}

void glapp_bind_sdl_events(GLApp& app, std::function<void(SDL_Event)> hook) {
    app.SDL_event_hook = hook;
}