#include <SDL2/SDL.h>
#include <GL/glew.h>

int main(int argc, char* argv[])
{
    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);

    // Create a window
    SDL_Window* window = SDL_CreateWindow(
        "OpenGL Window",                  // window title
        SDL_WINDOWPOS_UNDEFINED,          // initial x position
        SDL_WINDOWPOS_UNDEFINED,          // initial y position
        800,                              // width in pixels
        600,                              // height in pixels
        SDL_WINDOW_OPENGL                 // flags
    );

    // Create an OpenGL context
    SDL_GLContext context = SDL_GL_CreateContext(window);

    // Initialize GLEW (OpenGL Extension Wrangler)
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        // Handle GLEW initialization error
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        return 1;
    }

    // Set OpenGL viewport
    glViewport(0, 0, 800, 600);

    // OpenGL rendering code goes here...

    bool quit = false;
    while (!quit)
    {
        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
        }

        // OpenGL rendering code goes here...
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Swap buffers
        SDL_GL_SwapWindow(window);
    }

    // Clean up
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
