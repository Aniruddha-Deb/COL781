#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <omp.h>

#include "constants.hpp"
#include "renderer.hpp"
#include "debug.hpp"

#define vec4_to_color(fmt, color)                                                                                      \
    SDL_MapRGBA(fmt, (Uint8)(color[0] * 255), (Uint8)(color[1] * 255), (Uint8)(color[2] * 255), (Uint8)(color[3] * 255))

#define vec3_to_color(fmt, color)                                                                                      \
    SDL_MapRGBA(fmt, (Uint8)(color[0] * 255), (Uint8)(color[1] * 255), (Uint8)(color[2] * 255), (Uint8)(255))

glm::vec3 tone_map(glm::vec3 color)
{
    return glm::min(glm::vec3(1.f, 1.f, 1.f), color);
}

Renderer::Renderer(Window &_w, Scene &_s, int _spp, bool _path_traced)
    : win{_w}, scene{_s}, spp{_spp}, path_traced{_path_traced}, curr_sample_no{0}
{
    framebuffer = SDL_CreateRGBSurface(0, win.w, win.h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0);
    if (path_traced)
    {
        samplebuffer = std::vector<glm::vec3>(win.w * win.h, glm::vec3(0.f, 0.f, 0.f));
    }
}

Renderer::~Renderer()
{
    SDL_FreeSurface(framebuffer);
}

void Renderer::render()
{
    SDL_LockSurface(framebuffer);
    Uint32 *pixels = (Uint32 *)framebuffer->pixels;
    SDL_PixelFormat *format = framebuffer->format;
    for (int px = 0; px < win.w; px++)
    {
        #pragma omp parallel for
        for (int py = 0; py < win.h; py++)
        {
            Ray r;
            glm::vec3 pxcolor;
            if (path_traced)
            {
                if (curr_sample_no > 1) {
                    r = scene.generate_ray(px, py, true);
                }
                else {
                    r = scene.generate_ray(px, py, false);
                }
                glm::vec3 pathcolor = scene.trace_path(r);
                samplebuffer[py*win.w + px] = (samplebuffer[py * win.w + px]*float(curr_sample_no) + pathcolor) / float(curr_sample_no + 1);
                pxcolor = tone_map(samplebuffer[py*win.w + px]);
            }
            else
            {
                r = scene.generate_ray(px, py);
                pxcolor = tone_map(scene.trace_ray(r));
            }
            pixels[py * win.w + px] = vec3_to_color(format, pxcolor);
        }
    }
    SDL_UnlockSurface(framebuffer);
    if (path_traced)
    {
        curr_sample_no++;
    }
}

void Renderer::pan(float deltaPosX, float deltaPosY)
{
    Camera &camera = scene.camera;
    glm::vec4 pivot = glm::vec4(camera.lookAt.x, camera.lookAt.y, camera.lookAt.z, 1.0f);
    glm::vec4 position = glm::vec4(camera.position.x, camera.position.y, camera.position.z, 1.0f);

    // float deltaAngleY = CAMERA_DELTA_ANGLE_Y;
    float xAngle = (float)(deltaPosX)*CAMERA_DELTA_ANGLE_X;
    float yAngle = (float)(deltaPosY)*CAMERA_DELTA_ANGLE_Y;

    float cosAngle = dot(camera.getViewDir(), camera.up);

    if (cosAngle * signbit(yAngle) > 0.99f)
        yAngle = 0.0f;

    glm::mat4 rotationMatX(1.0f);
    rotationMatX = glm::rotate(rotationMatX, xAngle, camera.up);
    position = (rotationMatX * (position - pivot)) + pivot;

    glm::mat4 rotationMatY(1.0f);
    rotationMatY = glm::rotate(rotationMatY, yAngle, camera.getRightVector());
    glm::vec3 finalPosition = (rotationMatY * (position - pivot)) + pivot;
    camera.position = finalPosition;
    camera.updateViewMatrix();
}

void Renderer::move(float delta)
{
    Camera &camera = scene.camera;
    float deltaY = (float)(delta) * 0.01f;
    glm::mat4 dollyTransform = glm::mat4(1.0f);
    dollyTransform = glm::translate(dollyTransform, normalize(camera.lookAt - camera.position) * deltaY);
    glm::vec3 newCameraPosition = dollyTransform * glm::vec4(camera.position, 1.0f);

    if (signbit(newCameraPosition.z) == signbit(camera.position.z))
    {
        camera.position = newCameraPosition;
    }
}

void Renderer::reset_samplebuffer()
{
    for (auto &v : samplebuffer)
    {
        v = glm::vec3(0.f, 0.f, 0.f);
    }
    curr_sample_no = 0;
    cdebug << "resetting samplebuffer\n";
}

std::pair<int, int> Renderer::handle_events(int lastxPos, int lastyPos)
{

    int xPos, yPos;
    Uint32 buttonState = SDL_GetMouseState(&xPos, &yPos);
    bool dirty = false;
    if (buttonState & SDL_BUTTON(SDL_BUTTON_LEFT))
    {
        pan(float(lastxPos - xPos), float(lastyPos - yPos));
        dirty = true;
    }
    buttonState = SDL_GetMouseState(&xPos, &yPos);
    if (buttonState & SDL_BUTTON(SDL_BUTTON_RIGHT))
    {
        move(float(lastyPos - yPos));
        dirty = true;
    }
    if (dirty && path_traced)
    {
        reset_samplebuffer();
    }
    return {xPos, yPos};
}

void Renderer::view()
{
    int lastxPos, lastyPos;
    SDL_GetMouseState(&lastxPos, &lastyPos);
    while (!win.should_quit())
    {
        scene.camera.updateViewMatrix();
        auto [newxPos, newyPos] = handle_events(lastxPos, lastyPos);
        lastxPos = newxPos;
        lastyPos = newyPos;

        render();
        win.blit_surface(framebuffer);
    }
}
