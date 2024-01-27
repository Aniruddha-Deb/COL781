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
    _alive{true},
    _work(n_threads, false),
    _work_lock(n_threads),
    _work_write_lock(n_threads),
    _tri{},
    _fn{} {}

RasterizerThreadPool::~RasterizerThreadPool() {
    if (_alive) {
        this->stop();
    }
}

void RasterizerThreadPool::_thread_fn(int index) {
    while (_alive) {
        if (!_work[index]) continue; 
        // do work
        while (!_workqueues[index].empty()) {
            auto [tl, br] = _workqueues[index].back();
            _fn(index, _r, _tri, tl, br);
            _workqueues[index].pop_back();
        }
        // turn self off
        _work[index] = false;
    }
}

void RasterizerThreadPool::start() {
    if (!_alive) {
        _alive = true;
        for (int i=0; i<_n_threads; i++) {
            // TODO CPU affinity - not so easy to set in a cross-platform manner.
            // Especially hard (impossible?) to do on MacOS w/ arm processors
            _threads[i] = std::thread([this, i] { this->_thread_fn(i); } );
        }
    }
}

void RasterizerThreadPool::run() {
    // all of _work[i] guaranteed to be false here
    for (int i=0; i<_n_threads; i++) {
        _work[i] = true;
    }

    // see the work status
    bool all_free = false;
    while (!all_free) {
        all_free = true;
        for (int i=0; i<_n_threads; i++) {
            all_free = (all_free && (!_work[i]));
        }
    }
}

void RasterizerThreadPool::stop() {
    _alive = false;
    for (int i=0; i<_n_threads; i++) {
        _threads[i].join();
    }
}

// obviously don't call this after run has been called!
void RasterizerThreadPool::enqueue(glm::ivec2 tl, glm::ivec2 br) {
    _workqueues[_next_workq].push_back({tl, br});
    _next_workq = (_next_workq+1)%_n_threads;
}

void RasterizerThreadPool::set_render_function(std::function<void(int,Rasterizer*,glm::vec2(&)[3],glm::ivec2&,glm::ivec2&)> fn) {
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
    fb = new Uint32[w*h];
    rtp = new RasterizerThreadPool(this, 4);
}

Rasterizer::~Rasterizer() {
    delete rtp;
    delete fb;
}

int Rasterizer::width() {
    return _w;
}

int Rasterizer::height() {
    return _h;
}

void Rasterizer::clear(Uint32 color) {
    for (int i=0; i<_w*_h; i++) fb[i] = color;
}

void* Rasterizer::get_framebuffer() {
    return fb;
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

// need a sort of uniforms implementation (&tri, color go there)
void rasterize_block(int tid, Rasterizer *r, glm::vec2 (&tri)[3], glm::ivec2& tl, glm::ivec2& br) {

    int _w = r->width();
    int _h = r->height();
    glm::vec2 p(1.0f/_w, 1.0f/_h);

    // square ignore test: 
    //    for all edges of the triangle 
    //        check if the other point of the triangle is on the opposite side 
    //        compared to all four points of the square

    glm::vec2 p1 = pix2pt(tl.x, tl.y, p);
    glm::vec2 p2 = pix2pt(tl.x, br.y, p);
    glm::vec2 p3 = pix2pt(br.x, br.y, p);
    glm::vec2 p4 = pix2pt(br.x, tl.y, p);

    for (int k=0; k<3; k++) {
        auto v1 = tri[k%3];
        auto v2 = tri[(k+1)%3];
        auto v3 = tri[(k+2)%3];

        auto s = v2-v1;
        glm::vec2 n(-s.y, s.x);
        float t = glm::dot(n, v3-v1);

        float d1 = glm::dot((p1-v1),n);
        float d2 = glm::dot((p2-v1),n);
        float d3 = glm::dot((p3-v1),n);
        float d4 = glm::dot((p4-v1),n);

        if (d1*t < 0 && d2*t < 0 && d3*t < 0 && d4*t < 0) {
            // tri outside sq
            return;
        }
    }

    for (int y = std::max(0, tl.y); y <= std::min(_h-1, br.y); y++) {
        for (int x = std::max(0, tl.x); x <= std::min(_w-1, br.x); x++) {
            glm::vec2 pc = pix2pt(x, y, p);
            glm::vec3 p = phi(tri, pc);

            if (p[0] >= 0 && p[1] >= 0 && p[2] >= 0) {
                r->fb[_w*(_h-1-y) + x] = 0x00FF0000;
            }
        }
    }
}

void Rasterizer::rasterize(glm::vec2 (&tri)[3], Uint32 color) {

    glm::vec2 p(1.0f/_w, 1.0f/_h);

    float xmin = fminf(tri[0].x, fminf(tri[1].x, tri[2].x)), ymin = fminf(tri[0].y, fminf(tri[1].y, tri[2].y));
    float xmax = fmaxf(tri[0].x, fmaxf(tri[1].x, tri[2].x)), ymax = fmaxf(tri[0].y, fmaxf(tri[1].y, tri[2].y));

    glm::ivec2 tl = pt2pix(xmin-p.x, ymin-p.y, p);
    glm::ivec2 br = pt2pix(xmax+p.x, ymax+p.y, p);

    int cw=16, ch=16; // divisible by total dim

    rtp->set_triangle(tri);
    rtp->set_render_function(rasterize_block);

    // chunk this up and enqueue
    for (int i=(tl.y/ch)*ch; i<=(br.y/ch)*ch; i+=ch) {
        for (int j=(tl.x/cw)*cw; j<=(br.x/cw)*cw; j+=cw) {
            rtp->enqueue(glm::ivec2(j,i), glm::ivec2(j+cw-1,i+cw-1));
        }
    }

    rtp->run();

    /*
    for (int y = std::max(0, tl.y); y <= std::min(_h-1, br.y); y++) {
        for (int x = std::max(0, tl.x); x <= std::min(_w-1, br.x); x++) {
            glm::vec2 pc = pix2pt(x, y, p);
            glm::vec3 p = phi(tri, pc);

            if (p[0] >= 0 && p[1] >= 0 && p[2] >= 0) {
                _fb[_w*(_h-1-y) + x] = color;
            }
        }
    }
    */
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

    SDL_UpdateTexture(texture, NULL, fb, 4*_w);

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
    r.rtp->start();

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
    r.rtp->stop();
}

void rasterize_test_fn(int tid, Rasterizer *r, glm::vec2 (&tri)[3], glm::ivec2& tl, glm::ivec2& br) {
    std::cout << tid << "\n";
}

void thread_pool_test() {
    RasterizerThreadPool r(nullptr, 4);
    glm::vec2 tri[3] = {
        glm::vec2(-0.3, 0.6),
        glm::vec2(-0.8, 0.4),
        glm::vec2(0.5, -0.4)
    };

    r.start();
    r.set_triangle(tri);
    r.set_render_function(rasterize_test_fn);
    r.enqueue(glm::ivec2(0,0), glm::ivec2(15,15));
    r.enqueue(glm::ivec2(0,16), glm::ivec2(15,31));
    r.enqueue(glm::ivec2(0,32), glm::ivec2(15,47));
    r.enqueue(glm::ivec2(0,48), glm::ivec2(15,63));
    r.enqueue(glm::ivec2(0,0), glm::ivec2(15,15));
    r.enqueue(glm::ivec2(0,16), glm::ivec2(15,31));
    r.enqueue(glm::ivec2(0,32), glm::ivec2(15,47));
    r.enqueue(glm::ivec2(0,48), glm::ivec2(15,63));
    r.run();
    r.stop();
}

int main(int argc, char** argv) {

    Rasterizer r(640, 480);
    r.clear(0x22222200);
    /*
    glm::vec2 tri[3] = {
        glm::vec2(-0.3, 0.6),
        glm::vec2(-0.8, 0.4),
        glm::vec2(0.5, -0.4)
    };
    r.rasterize(tri, 0xFF000000);
    */
    benchmark(r, 1000);

    r.display();
    
    // thread_pool_test();

    return 0;
}
