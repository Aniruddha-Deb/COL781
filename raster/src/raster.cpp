// Rasterization starter code for COL781: Computer Graphics

/* Instructions for execution:
   1. Install SDL2 and SDL2_image libraries
   2. Compile using: g++ starter_code.cpp -I/path/to/SDL2 -lSDL2 -lSDL2_image
   (on Linux or MacOS, it should be sufficient to copy-paste the following:
   g++ starter_code.cpp `pkg-config --cflags --libs SDL2 SDL2_image`
   3. Run using: ./a.out
   4. The rendered frame will be shown in a window, and saved to a file on exit
*/

#include <stdio.h>
#include <algorithm>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glm/glm.hpp>

/* Framebuffer-related variables.
     We create a 10x10 image and display it at 40x size so you can clearly
     see the pixels! Set displayScale to 1 to see it at the correct size.
     Then you can increase frameWidth and frameHeight as desired. */

const int frameWidth = 80;
const int frameHeight = 80;

const int displayScale = 5;

SDL_Surface *framebuffer = NULL;

/* SDL parameters */

SDL_Window *window = NULL;
SDL_Surface *windowSurface = NULL;
bool quit = false;

/* Output file */

const char *outputFile = "out.png";

/* Function prototypes */

bool initialize();
void handleEvents();
void saveFramebuffer();
void terminate();

bool membership_check(glm::vec2 (&triangle)[3], glm::vec2 &pt)
{
    for (int k = 0; k < 3; k++)
    {
        glm::vec2 v1 = triangle[k % 3];
        glm::vec2 v2 = triangle[(k + 1) % 3];
        glm::vec2 s = v2 - v1;
        glm::vec2 t(-s.y, s.x);

        float dot = glm::dot(t, pt - v1);
        if (dot < 0)
        {
            return false;
        }
    }
    return true;
}

glm::vec2 pix_to_pt(int x, int y)
{
    return glm::vec2((float(x) + 0.5) / frameWidth, (float(y) + 0.5) / frameHeight);
}

Uint32 pix_blend(Uint32 new_pix, Uint32 old_pix)
{
    SDL_PixelFormat *format = framebuffer->format;
    Uint8 r_new, g_new, b_new, a_new;
    SDL_GetRGBA(new_pix, format, &r_new, &g_new, &b_new, &a_new);
    Uint8 r_old, g_old, b_old, a_old;
    SDL_GetRGBA(old_pix, format, &r_old, &g_old, &b_old, &a_old);
    Uint32 color;
    float alpha_new = a_new / (float)255, alpha_old = a_old / (float)255;
    r_new = r_new * alpha_new + r_old * (1 - alpha_new);
    g_new = g_new * alpha_new + g_old * (1 - alpha_new);
    b_new = b_new * alpha_new + b_old * (1 - alpha_new);
    a_new = 255 * (alpha_new + alpha_old * (1 - alpha_new));
    color = SDL_MapRGBA(format, r_new, g_new, b_new, a_new);
    return color;
}

int value_binary(glm::vec2 (&triangle)[3], int x, int y)
{
    glm::vec2 pt = pix_to_pt(x, y);
    if (membership_check(triangle, pt))
        return 255;
    else
        return -1;
}

float value_4xmsaa_rot(glm::vec2 (&triangle)[3], int x, int y)
{
    glm::vec2 pt1 = pix_to_pt(x, y) + glm::vec2(0.3 / frameWidth, 0.2 / frameHeight);
    glm::vec2 pt2 = pix_to_pt(x, y) + glm::vec2(-0.2 / frameWidth, 0.3 / frameHeight);
    glm::vec2 pt3 = pix_to_pt(x, y) + glm::vec2(-0.3 / frameWidth, -0.2 / frameHeight);
    glm::vec2 pt4 = pix_to_pt(x, y) + glm::vec2(0.2 / frameWidth, -0.3 / frameHeight);

    float alpha = 0;

    if (membership_check(triangle, pt1))
        alpha += 0.25;
    if (membership_check(triangle, pt2))
        alpha += 0.25;
    if (membership_check(triangle, pt3))
        alpha += 0.25;
    if (membership_check(triangle, pt4))
        alpha += 0.25;

    return alpha;
}

void render_background() {
    Uint32 *pixels = (Uint32 *)framebuffer->pixels;
    SDL_PixelFormat *format = framebuffer->format;
    Uint32 background;
    for (int i = 0; i < frameHeight; i++)
    {
        for (int j = 0; j < frameWidth; j++)
        {
            float l = (200.0 * i + 50) / frameHeight;
            background = SDL_MapRGBA(format, l, l, l, 255);
            pixels[(frameHeight - i - 1) * frameWidth + j] = background;
        }
    }
}

void render_naive(glm::vec2 (&triangle)[3])
{

    Uint32 *pixels = (Uint32 *)framebuffer->pixels;
    SDL_PixelFormat *format = framebuffer->format;

    for (int i = 0; i < frameHeight; i++)
    {
        for (int j = 0; j < frameWidth; j++)
        {
            Uint32 background = pixels[(frameHeight - i - 1) * frameWidth + j];
            Uint32 foreground;
            float v = value_4xmsaa_rot(triangle, j, i);
            foreground = SDL_MapRGBA(format, 0, 153, 0, 255 * v);
            pixels[(frameHeight - i - 1) * frameWidth + j] = pix_blend(foreground, background);
        }
    }
}

void render_triangles(std::vector<glm::vec2> &verts, std::vector<glm::vec3> &idxs) {

    render_background();
    for (auto idx : idxs) {
        glm::vec2 triangle[3] = {verts[idx.x], verts[idx.y], verts[idx.z]};
        render_naive(triangle);
    }
}

void render_original_circle()
{
    Uint32 *pixels = (Uint32 *)framebuffer->pixels;
    SDL_PixelFormat *format = framebuffer->format;
    for (int i = 0; i < frameWidth; i++)
    {
        for (int j = 0; j < frameHeight; j++)
        {
            float x = i + 0.5;
            float y = j + 0.5;
            float cx = 10, cy = 10, r = 8;
            Uint32 color;
            if ((x - cx) * (x - cx) + (y - cy) * (y - cy) <= r * r)
            {                                                // inside circle
                color = SDL_MapRGBA(format, 0, 153, 0, 255); // green
            }
            else
            {
                float l = 255.0 * j / frameHeight;
                color = SDL_MapRGBA(format, l, l, l, 255); // grey proportional to y
            }
            pixels[i + frameWidth * j] = color;
        }
    }
}

int main(int argc, char *args[])
{
    if (!initialize())
    {
        printf("Failed to initialize!");
    }
    else
    {
        // Display and interaction
        std::vector<glm::vec2> verts = {
            glm::vec2(0.1, 0.5),
            glm::vec2(0.4, 0.1),
            glm::vec2(0.9, 0.8),
            glm::vec2(0.4, 0.35)
        };
        std::vector<glm::vec3> idxs = {
            glm::vec3(0, 1, 3),
            glm::vec3(1, 2, 3)
        };
        while (!quit)
        {
            // Event handling
            handleEvents();

            /* Set pixel data.
               CHANGE THIS TO YOUR OWN CODE! */
            render_triangles(verts, idxs);

            // Update screen to apply the changes
            SDL_BlitScaled(framebuffer, NULL, windowSurface, NULL);
            SDL_UpdateWindowSurface(window);
        }
    }
    // Save image
    saveFramebuffer();
    terminate();
    return 0;
}

/* Everything below here is library-specific boilerplate code.
     You can ignore it for now. */

// Initialising SDL2
bool initialize()
{
    bool success = true;
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        success = false;
    }
    else
    {
        int screenWidth = frameWidth * displayScale;
        int screenHeight = frameHeight * displayScale;
        window = SDL_CreateWindow("COL781", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
        if (window == NULL)
        {
            printf("Window could not be created! SDL_Error: %s", SDL_GetError());
            success = false;
        }
        else
        {
            windowSurface = SDL_GetWindowSurface(window);
            framebuffer = SDL_CreateRGBSurface(0, frameWidth, frameHeight, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
        }
    }
    return success;
}

// Handle window exit
void handleEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT)
        {
            quit = true;
        }
    }
}

void saveFramebuffer()
{
    // Save the image
    IMG_SavePNG(framebuffer, outputFile);
}

void terminate()
{
    // Free resources and close SDL
    SDL_FreeSurface(framebuffer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
