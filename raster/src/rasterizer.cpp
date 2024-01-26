#include "rasterizer.hpp"
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <cmath>

RasterizerThreadPool::RasterizerThreadPool(Rasterizer *r, size_t n_threads) : 
    _r{r},
    _n_threads(n_threads),
    _threads(n_threads),
    _workqueues(n_threads, std::vector<std::pair<glm::ivec2, glm::ivec2>>()),
    _next_workq{0},
    _work(n_threads, false),
    _tri{},
    _fn{}
{
    for (int i=0; i<n_threads; i++) {
        _work_lock[i].lock();
    }
    for (int i=0; i<n_threads; i++) {
        _threads[i] = std::thread(&RasterizerThreadPool::_thread_fn, this, i);
    }

}

void RasterizerThreadPool::_thread_fn(int index) {
    while (true) {
        std::lock_guard l(_work_lock[index]);
        if (!_work[index]) continue; // offer a chance to the other thread to 
                                     // reacquire the lock
                                     // ideally a fifo semaphore here would've 
                                     // been best.
        // do work
        while (!_workqueues[index].empty()) {
            auto [tl, br] = _workqueues[index].back();
            _fn(_r, _tri, tl, br);
            _workqueues[index].pop_back();
        }
        // turn self off
        {
            std::lock_guard l(_work_write_lock[index]);
            _work[index] = false;
        }
    }
}

void RasterizerThreadPool::run() {
    // all of _work[i] guaranteed to be false here
    for (int i=0; i<_n_threads; i++) {
        std::lock_guard l(_work_write_lock[i]);
        _work[i] = true;
    }
    for (int i=0; i<_n_threads; i++) {
        _work_lock[i].unlock();
    }

    // see the work status
    bool all_free = true;
    while (!all_free) {
        for (int i=0; i<_n_threads; i++) {
            all_free = (all_free && _work[i]);
        }
    }

    // reacquire the work locks
    for (int i=0; i<_n_threads; i++) {
        _work_lock[i].lock();
    }
}

// obviously don't call this after run has been called!
void RasterizerThreadPool::enqueue(glm::ivec2& tl, glm::ivec2& br) {
    _workqueues[_next_workq].push_back({tl, br});
    _next_workq = (_next_workq+1)%_n_threads;
}

void RasterizerThreadPool::set_render_function(std::function<void(Rasterizer*,glm::vec2(&)[3],glm::ivec2&,glm::ivec2&)>& fn) {
    _fn = fn;
}

void RasterizerThreadPool::set_triangle(glm::vec2 (&tri)[3]) {
    _tri[0] = tri[0];
    _tri[1] = tri[1];
    _tri[2] = tri[2];
}

Rasterizer::Rasterizer(int w, int h) {
    _w = w;
    _h = h;
    _fb = new Uint32[w*h];
    _rtp = new RasterizerThreadPool(this, 4);
}

Rasterizer::~Rasterizer() {
    delete _rtp;
    delete _fb;
}

int Rasterizer::width() {
    return _w;
}

int Rasterizer::height() {
    return _h;
}

void Rasterizer::clear(Uint32 color) {
    for (int i=0; i<_w*_h; i++) _fb[i] = color;
}

void* Rasterizer::get_framebuffer() {
    return _fb;
}

#define pix2pt(x, y, p) glm::vec2((2*(x) + 1)*p[0] - 1, (2*(y) + 1)*p[1] - 1)
#define pt2pix(x, y, p) glm::ivec2(round(((x) + 1)/(2*p[0]) - 0.5f), round(((y) + 1)/(2*p[1]) - 0.5f))

glm::vec3 phi(glm::vec2 (&tri)[3], glm::vec2 pt) {
    float denominator = (tri[0].x * (tri[1].y - tri[2].y) + tri[0].y * (tri[2].x - tri[1].x) + tri[1].x * tri[2].y -
                         tri[1].y * tri[2].x);
    float t1 =
        (pt.x * (tri[2].y - tri[0].y) + pt.y * (tri[0].x - tri[2].x) - tri[0].x * tri[2].y + tri[0].y * tri[2].x) /
        denominator;
    float t2 =
        -(pt.x * (tri[1].y - tri[0].y) + pt.y * (tri[0].x - tri[1].x) - tri[0].x * tri[1].y + tri[0].y * tri[1].x) /
        denominator;
    float s = 1 - t1 - t2;
    return glm::vec3(s, t1, t2);
}

void Rasterizer::rasterize(glm::vec2 (&tri)[3], Uint32 color) {

    glm::vec2 p(1.0f/_w, 1.0f/_h);

    float xmin = fminf(tri[0].x, fminf(tri[1].x, tri[2].x)), ymin = fminf(tri[0].y, fminf(tri[1].y, tri[2].y));
    float xmax = fmaxf(tri[0].x, fmaxf(tri[1].x, tri[2].x)), ymax = fmaxf(tri[0].y, fmaxf(tri[1].y, tri[2].y));

    glm::ivec2 tl = pt2pix(xmin-p.x, ymin-p.y, p);
    glm::ivec2 br = pt2pix(xmax+p.x, ymax+p.y, p);

    for (int y = std::max(0, tl.y); y <= std::min(_h-1, br.y); y++) {
        for (int x = std::max(0, tl.x); x <= std::min(_w-1, br.x); x++) {
            glm::vec2 pc = pix2pt(x, y, p);
            glm::vec3 p = phi(tri, pc);

            if (p[0] >= 0 && p[1] >= 0 && p[2] >= 0) {
                _fb[_w*(_h-1-y) + x] = color;
            }
        }
    }
}

// need a sort of uniforms implementation (&tri, color go there)
void rasterize_block(Rasterizer *r, glm::vec2 (&tri)[3], glm::ivec2& tl, glm::ivec2& br) {

    int _w = r->width();
    int _h = r->height();
    glm::vec2 p(1.0f/_w, 1.0f/_h);

    for (int y = std::max(0, tl.y); y <= std::min(_h-1, br.y); y++) {
        for (int x = std::max(0, tl.x); x <= std::min(_w-1, br.x); x++) {
            glm::vec2 pc = pix2pt(x, y, p);
            glm::vec3 p = phi(tri, pc);

            if (p[0] >= 0 && p[1] >= 0 && p[2] >= 0) {
                r->_fb[_w*(_h-1-y) + x] = 0x00FF0000;
            }
        }
    }
}

int Rasterizer::display() {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create a window
    SDL_Window* window = SDL_CreateWindow("Framebuffer Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _w, _h, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create a texture for rendering
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, _w, _h);
    if (!texture) {
        printf("Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_UpdateTexture(texture, NULL, _fb, 4*_w);

    // Render the texture to the screen
    while (true) {
        SDL_Event event;
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                break;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    // Clean up resources
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

// tests

static std::random_device rd;
static std::mt19937 rng{rd()}; 


void benchmark(Rasterizer& r, int n_tris) {

    using namespace std::chrono;

    static std::uniform_real_distribution<double> dist(-1,1);
    static std::uniform_int_distribution<uint32_t> cdist(0x0100, 0xFFFFFF00);

    auto tic = high_resolution_clock::now();
    for (int i=0; i<n_tris; i++) {

        glm::vec2 tri[3] = {
            glm::vec2(dist(rng), dist(rng)),
            glm::vec2(dist(rng), dist(rng)),
            glm::vec2(dist(rng), dist(rng))
        };

        r.rasterize(tri, cdist(rng));
    }
    auto toc = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(toc - tic).count();
    std::cout << "Render " << n_tris << " tris in " << duration << " us (" 
              << (1000000.f*n_tris/duration) << " tris/sec)" << std::endl;
}

void thread_pool_test() {
    RasterizerThreadPool r(nullptr, 4);

    // TODO see if the processing is happening.
}

int main(int argc, char** argv) {

    Rasterizer r(640, 480);
    r.clear(0x22222200);
    benchmark(r, 1000);

    r.display();

    return 0;
}
