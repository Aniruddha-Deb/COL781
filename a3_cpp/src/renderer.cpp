#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer.hpp"
#include "debug.hpp"

#define vec4_to_color(fmt, color)                                                                                      \
    SDL_MapRGBA(fmt, (Uint8)(color[0] * 255), (Uint8)(color[1] * 255), (Uint8)(color[2] * 255), (Uint8)(color[3] * 255))

#define vec3_to_color(fmt, color)                                                                                      \
    SDL_MapRGBA(fmt, (Uint8)(color[0] * 255), (Uint8)(color[1] * 255), (Uint8)(color[2] * 255), (Uint8)(255))

Renderer::Renderer(Window &_w, Scene &_s, int _spp) : win{_w}, scene{_s}, spp{_spp}
{
    framebuffer = SDL_CreateRGBSurface(0, win.w, win.h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0);
}

Renderer::~Renderer()
{
    SDL_FreeSurface(framebuffer);
}

void Renderer::render()
{
    // TODO possibly parallelize
    SDL_LockSurface(framebuffer);
    Uint32 *pixels = (Uint32 *)framebuffer->pixels;
    SDL_PixelFormat *format = framebuffer->format;
    for (int px = 0; px < win.w; px++)
    {
        for (int py = 0; py < win.h; py++)
        {
            Ray r = scene.generate_ray(px, py);
            glm::vec3 pxcolor = scene.trace_ray(r);
            pixels[py * win.w + px] = vec3_to_color(format, pxcolor);
        }
    }
    SDL_UnlockSurface(framebuffer);
}

void Renderer::view()
{

    // The transformation matrix.
    glm::mat4 view;
    Camera &camera = scene.camera;

    float deltaAngleX = 2.0 * 3.14 / 800.0;
    float deltaAngleY = 3.14 / 600.0;

    int lastxPos, lastyPos, xPos, yPos;

    SDL_GetMouseState(&lastxPos, &lastyPos);

    while (!win.should_quit())
    {

        camera.updateViewMatrix();

        Uint32 buttonState = SDL_GetMouseState(&xPos, &yPos);
        if (buttonState & SDL_BUTTON(SDL_BUTTON_LEFT))
        {
            glm::vec4 pivot = glm::vec4(camera.lookAt.x, camera.lookAt.y, camera.lookAt.z, 1.0f);
            glm::vec4 position = glm::vec4(camera.position.x, camera.position.y, camera.position.z, 1.0f);

            float xAngle = (float)(lastxPos - xPos) * deltaAngleX;
            float yAngle = (float)(lastyPos - yPos) * deltaAngleY;

            float cosAngle = dot(camera.getViewDir(), camera.up);

            if (cosAngle * signbit(deltaAngleY) > 0.99f)
                deltaAngleY = 0.0f;

            glm::mat4 rotationMatX(1.0f);
            rotationMatX = glm::rotate(rotationMatX, xAngle, camera.up);
            position = (rotationMatX * (position - pivot)) + pivot;

            glm::mat4 rotationMatY(1.0f);
            rotationMatY = glm::rotate(rotationMatY, yAngle, camera.getRightVector());
            glm::vec3 finalPosition = (rotationMatY * (position - pivot)) + pivot;
            camera.position = finalPosition;
            camera.updateViewMatrix();
        }

        buttonState = SDL_GetMouseState(&xPos, &yPos);
        if (buttonState & SDL_BUTTON(SDL_BUTTON_RIGHT))
        {
            // Update camera parameters

            float deltaY = (float)(lastyPos - yPos) * 0.01f;
            glm::mat4 dollyTransform = glm::mat4(1.0f);
            dollyTransform = glm::translate(dollyTransform, normalize(camera.lookAt - camera.position) * deltaY);
            glm::vec3 newCameraPosition = dollyTransform * glm::vec4(camera.position, 1.0f);

            if (signbit(newCameraPosition.z) == signbit(camera.position.z))
            {
                camera.position = newCameraPosition;
            }
        }

        lastxPos = xPos;
        lastyPos = yPos;

        view = camera.getViewMatrix();

        render();
        win.blit_surface(framebuffer);
    }
}
