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

const int frameWidth = 40;
const int frameHeight = 40;

const int displayScale = 10;

SDL_Surface* framebuffer = NULL;

/* SDL parameters */

SDL_Window* window = NULL;
SDL_Surface *windowSurface = NULL;
bool quit = false;

/* Output file */

const char* outputFile = "out.png";

/* Function prototypes */

bool initialize();
void handleEvents();
void saveFramebuffer();
void terminate();

bool membership_check(glm::vec2 (&triangle)[3], glm::vec2 &pt) {
    for (int k=0; k<3; k++) {
        glm::vec2 v1 = triangle[k%3];
        glm::vec2 v2 = triangle[(k+1)%3];
        glm::vec2 s = v2-v1;
        glm::vec2 t(-s.y, s.x);

        float dot = glm::dot(t, pt-v1);
        if (dot < 0) {
            return false;
        }
    }
    return true;
}

glm::vec2 pix_to_pt(int x, int y) {
    return glm::vec2((float(x)+0.5)/frameWidth, (float(y)+0.5)/frameHeight);
}

int value_binary(glm::vec2 (&triangle)[3], int x, int y) {
    glm::vec2 pt = pix_to_pt(x, y);
    if (membership_check(triangle, pt)) return 255;
    else return -1;
}

float value_4xmsaa_rot(glm::vec2 (&triangle)[3], int x, int y) {
    glm::vec2 pt1 = pix_to_pt(x, y) + glm::vec2(0.3/frameWidth, 0.2/frameHeight);
    glm::vec2 pt2 = pix_to_pt(x, y) + glm::vec2(-0.2/frameWidth, 0.3/frameHeight);
    glm::vec2 pt3 = pix_to_pt(x, y) + glm::vec2(-0.3/frameWidth, -0.2/frameHeight);
    glm::vec2 pt4 = pix_to_pt(x, y) + glm::vec2(0.2/frameWidth, -0.3/frameHeight);

    float alpha = 0;

    if (membership_check(triangle, pt1)) alpha += 0.25;
    if (membership_check(triangle, pt2)) alpha += 0.25;
    if (membership_check(triangle, pt3)) alpha += 0.25;
    if (membership_check(triangle, pt4)) alpha += 0.25;

    return alpha; // figure out blending @Salil
}

void render_naive(glm::vec2 (&triangle)[3]) {

    Uint32 *pixels = (Uint32*)framebuffer->pixels;
    SDL_PixelFormat *format = framebuffer->format;

    for (int i=0; i<frameHeight; i++) {
        for (int j=0; j<frameWidth; j++) {
            float l = (200.0*i + 50)/frameHeight;
            pixels[(frameHeight-i-1)*frameWidth + j] = SDL_MapRGBA(format, l, l, l, 255);
        }
    }

    for (int i=0; i<frameHeight; i++) {
        for (int j=0; j<frameWidth; j++) {
            Uint32 color;
            int v = value_4xmsaa_rot(triangle, j, i);
            color = SDL_MapRGBA(format, 0, 153, 0, v*255);
            // TODO blending
            pixels[(frameHeight-i-1)*frameWidth + j] = color;
        }
    }
}

void render_original_circle() {
    Uint32 *pixels = (Uint32*)framebuffer->pixels;
    SDL_PixelFormat *format = framebuffer->format;
    for (int i = 0; i < frameWidth; i++) {
        for (int j = 0; j < frameHeight; j++) {
            float x = i + 0.5;
            float y = j + 0.5;
            float cx = 10, cy = 10, r = 8;
            Uint32 color;
            if ((x-cx)*(x-cx) + (y-cy)*(y-cy) <= r*r) { // inside circle
                color = SDL_MapRGBA(format, 0, 153, 0, 255); // green
            } else {
                float l = 255.0*j/frameHeight;
                color = SDL_MapRGBA(format, l, l, l, 255); // grey proportional to y
            }
            pixels[i + frameWidth*j] = color;
        }
    }
}

int main(int argc, char* args[]) {
    if (!initialize()) {
        printf("Failed to initialize!");
    } else {
        // Display and interaction
        while (!quit) {
            // Event handling
            handleEvents();

            /* Set pixel data.
               CHANGE THIS TO YOUR OWN CODE! */
            glm::vec2 triangle[3] = { glm::vec2(0.1, 0.1), glm::vec2(0.9, 0.1), glm::vec2(0.5, 0.9) };
            render_naive(triangle);

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
bool initialize() {
    bool success = true;
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        success = false;
    } else {
        int screenWidth = frameWidth * displayScale;
        int screenHeight = frameHeight * displayScale;
        window = SDL_CreateWindow("COL781", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
        if (window == NULL) {
            printf("Window could not be created! SDL_Error: %s", SDL_GetError());
            success = false;
        } else {
            windowSurface = SDL_GetWindowSurface(window);
            framebuffer = SDL_CreateRGBSurface(0, frameWidth, frameHeight, 32, 0, 0, 0, 0);
        }
    }
    return success;
}

// Handle window exit 
void handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
        }
    }
}
		
void saveFramebuffer() {
    // Save the image
    IMG_SavePNG(framebuffer, outputFile);
}

void terminate() {
    // Free resources and close SDL
    SDL_FreeSurface(framebuffer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
