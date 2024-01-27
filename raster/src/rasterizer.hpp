#pragma once

#include <glm/glm.hpp>
#include <functional>
#include <mutex>
#include <SDL2/SDL.h>

class Rasterizer;

class RasterizerThreadPool {
    public:
        RasterizerThreadPool(Rasterizer* r, size_t n_threads);
        ~RasterizerThreadPool();
        void run();
        void start();
        void stop();
        void enqueue(glm::ivec2 tl, glm::ivec2 br);
        void set_render_function(std::function<void(int,Rasterizer*,glm::vec2(&)[3],glm::ivec2&,glm::ivec2&)> fn);
        void set_triangle(glm::vec2 (&tri)[3]);

    private:
        void _thread_fn(int thread_idx);

        Rasterizer *_r;
        size_t _n_threads;
        std::vector<std::thread> _threads;
        std::vector<std::vector<std::pair<glm::ivec2,glm::ivec2>>> _workqueues;
        int _next_workq;
        volatile bool _alive;
        std::vector<bool> _work;
        std::vector<bool> _relax;
        std::vector<std::mutex> _work_lock;
        std::vector<std::mutex> _work_write_lock;
        glm::vec2 _tri[3];
        std::function<void(int, Rasterizer*, glm::vec2(&)[3], glm::ivec2&, glm::ivec2&)> _fn;
};

class Rasterizer {

    public:
    Rasterizer(int w, int h);
    ~Rasterizer();
    void rasterize(glm::vec2 (&tri)[3], Uint32 color);
    void clear(Uint32 color);
    void* get_framebuffer();
    int width();
    int height();

    int display();
    Uint32* fb;
    RasterizerThreadPool *rtp;

    private:
    int _w, _h;
};

