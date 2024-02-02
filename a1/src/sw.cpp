#include "SDL2/SDL_pixels.h"
#define GLM_SWIZZLE
#include "sw.hpp"

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

namespace COL781
{
namespace Software
{

    std::mutex mtx;

    ////////////////////////////////////////////////////////////////////////////
    /// Forward declarations
    ////////////////////////////////////////////////////////////////////////////

    template <> float Attribs::get(int index) const;
    template <> glm::vec2 Attribs::get(int index) const;
    template <> glm::vec3 Attribs::get(int index) const;
    template <> glm::vec4 Attribs::get(int index) const;

    template <> void Attribs::set(int index, float value);
    template <> void Attribs::set(int index, glm::vec2 value);
    template <> void Attribs::set(int index, glm::vec3 value);
    template <> void Attribs::set(int index, glm::vec4 value);

    ////////////////////////////////////////////////////////////////////////////
    /// Built-in shaders
    ////////////////////////////////////////////////////////////////////////////

    VertexShader Rasterizer::vsIdentity()
    {
        return [](const Uniforms &uniforms, const Attribs &in, Attribs &out) {
            glm::vec4 vertex = in.get<glm::vec4>(0);
            return vertex;
        };
    }

    VertexShader Rasterizer::vsTransform()
    {
        return [](const Uniforms &uniforms, const Attribs &in, Attribs &out) {
            glm::vec4 vertex = in.get<glm::vec4>(0);
            glm::mat4 transform = uniforms.get<glm::mat4>("transform");
            return transform * vertex;
        };
    }

    VertexShader Rasterizer::vsColor()
    {
        return [](const Uniforms &uniforms, const Attribs &in, Attribs &out) {
            glm::vec4 vertex = in.get<glm::vec4>(0);
            glm::vec4 color = in.get<glm::vec4>(1);
            out.set<glm::vec4>(0, color);
            return vertex;
        };
    }

    // Dont' know why this didn't come implemnted.
    VertexShader Rasterizer::vsColorTransform()
    {
        return [](const Uniforms &uniforms, const Attribs &in, Attribs &out) {
            glm::vec4 vertex = in.get<glm::vec4>(0);
            glm::vec4 color = in.get<glm::vec4>(1);
            glm::mat4 transform = uniforms.get<glm::mat4>("transform");
            out.set<glm::vec4>(0, color);
            return transform * vertex;
        };
    }

    FragmentShader Rasterizer::fsConstant()
    {
        return [](const Uniforms &uniforms, const Attribs &in) {
            glm::vec4 color = uniforms.get<glm::vec4>("color");
            return color;
        };
    }

    FragmentShader Rasterizer::fsIdentity()
    {
        return [](const Uniforms &uniforms, const Attribs &in) {
            glm::vec4 color = in.get<glm::vec4>(0);
            return color;
        };
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Attribs and Uniforms classes
    ////////////////////////////////////////////////////////////////////////////

    void checkDimension(int index, int actual, int requested)
    {
        if (actual != requested)
        {
            std::cout << "Warning: attribute " << index << " has dimension " << actual << " but accessed as dimension "
                      << requested << std::endl;
        }
    }

    void expand(std::vector<int> &dims, std::vector<glm::vec4> &values, int index)
    {
        if (dims.size() < index + 1)
            dims.resize(index + 1);
        if (values.size() < index + 1)
            values.resize(index + 1);
    }

    // clang-format off
    int Attribs::size() { return values.size(); }

    template <> float     Attribs::get(int index) const { checkDimension(index, dims[index], 1); return values[index].x; }
    template <> glm::vec2 Attribs::get(int index) const { checkDimension(index, dims[index], 2); return glm::vec2(values[index].x, values[index].y); }
    template <> glm::vec3 Attribs::get(int index) const { checkDimension(index, dims[index], 3); return glm::vec3(values[index].x, values[index].y, values[index].z); }
    template <> glm::vec4 Attribs::get(int index) const { checkDimension(index, dims[index], 4); return values[index]; }

    template <> void Attribs::set(int index, float     value) { expand(dims, values, index); dims[index] = 1; values[index] = glm::vec4(value, 0, 0, 0); }
    template <> void Attribs::set(int index, glm::vec2 value) { expand(dims, values, index); dims[index] = 2; values[index] = glm::vec4(value, 0, 0); }
    template <> void Attribs::set(int index, glm::vec3 value) { expand(dims, values, index); dims[index] = 3; values[index] = glm::vec4(value, 0); }
    template <> void Attribs::set(int index, glm::vec4 value) { expand(dims, values, index); dims[index] = 4; values[index] = value; }

    // Uniforms::get, Uniforms::set in sw.hpp
    // Type constraining setUniform down here.

    template <> void Rasterizer::setUniform(ShaderProgram &sp, const std::string &name, float     value) { sp.uniforms.set<float>    (name, value); }
    template <> void Rasterizer::setUniform(ShaderProgram &sp, const std::string &name, int       value) { sp.uniforms.set<int>      (name, value); }
    template <> void Rasterizer::setUniform(ShaderProgram &sp, const std::string &name, glm::vec2 value) { sp.uniforms.set<glm::vec2>(name, value); }
    template <> void Rasterizer::setUniform(ShaderProgram &sp, const std::string &name, glm::mat2 value) { sp.uniforms.set<glm::mat2>(name, value); }
    template <> void Rasterizer::setUniform(ShaderProgram &sp, const std::string &name, glm::vec3 value) { sp.uniforms.set<glm::vec3>(name, value); }
    template <> void Rasterizer::setUniform(ShaderProgram &sp, const std::string &name, glm::mat3 value) { sp.uniforms.set<glm::mat3>(name, value); }
    template <> void Rasterizer::setUniform(ShaderProgram &sp, const std::string &name, glm::vec4 value) { sp.uniforms.set<glm::vec4>(name, value); }
    template <> void Rasterizer::setUniform(ShaderProgram &sp, const std::string &name, glm::mat4 value) { sp.uniforms.set<glm::mat4>(name, value); }

    // clang-format on 

    ////////////////////////////////////////////////////////////////////////////
    /// Rasterizer windowing methods
    ////////////////////////////////////////////////////////////////////////////

    bool Rasterizer::initialize(const std::string &title, int width, int height, int spp)
    {
        bool success = true;
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        {
            printf("SDL could not initialize! SDL_Error: %s", SDL_GetError());
            success = false;
        }
        else
        {
            SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "1");
            int screenWidth = width;
            int screenHeight = height;
            window = SDL_CreateWindow("COL781", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth,
                                      screenHeight, SDL_WINDOW_SHOWN);
            if (window == NULL)
            {
                printf("Window could not be created! SDL_Error: %s", SDL_GetError());
                success = false;
            }
            else
            {
                int mult = sqrt(spp);
                framebuffer =
                    SDL_CreateRGBSurface(0, width*mult, height*mult, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0);
                    // SDL_CreateRGBSurface(0, width*mult, height*mult, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
            }
            rtp = new RasterizerThreadPool(6);
            rtp->framebuffer = &framebuffer;
            rtp->program = &shader_program;
            rtp->z_buffer = &z_buffer;
            // scaling interpolation 
        }
        return success;
    }

    bool Rasterizer::shouldQuit()
    {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                return true;
            }
        }
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Shader Program methods
    ////////////////////////////////////////////////////////////////////////////

    ShaderProgram Rasterizer::createShaderProgram(const VertexShader &vs, const FragmentShader &fs)
    {
        ShaderProgram sp;
        sp.vs = vs;
        sp.fs = fs;
        sp.uniforms = Uniforms();
        return sp;
    }

    void Rasterizer::useShaderProgram(const ShaderProgram &program)
    {
        shader_program = &program;
    }

    void Rasterizer::deleteShaderProgram(ShaderProgram &program)
    {
        if (&program == shader_program)
        {
            shader_program = nullptr;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Objects
    ////////////////////////////////////////////////////////////////////////////

    Object Rasterizer::createObject()
    {
        /*
        struct Object {
            using Buffer = std::vector<float>;
            std::vector<Buffer> attributeValues;
            std::vector<int> attributeDims;
            std::vector<glm::ivec3> indices;
        };
        */
        Object obj;
        return obj;
    }

    void printObject(const Object &obj)
    {
        std::cout << "Number of attribute buffers: " << obj.attributeValues.size() << "\n";

        int i = 0;
        for (const auto &buf : obj.attributeValues)
        {
            std::cout << "Attribute buffer " << i++ << " dimensions: ";
            if (!obj.attributeDims.empty())
            {
                std::cout << obj.attributeDims[i - 1];
                if (std::next(obj.attributeDims.begin(), i) != obj.attributeDims.end())
                {
                    std::cout << " x ";
                }
            }
            std::cout << "\n";
            for (float val : buf)
            {
                std::cout << val << " ";
            }
            std::cout << "\n\n";
        }
        std::cout << "Number of indices: " << obj.indices.size() << "\n";
        for (const glm::ivec3 &idx : obj.indices)
        {
            std::cout << "[ " << idx.x << ", " << idx.y << ", " << idx.z << " ]\n";
        }
    }

    void setAttribs(Object &object, int attribIndex, int n, int dim, const float *data)
    {
        if (object.attributeValues.size() <= attribIndex)
        {
            object.attributeValues.resize(attribIndex + 1);
        }
        if (object.attributeDims.size() <= attribIndex)
        {
            object.attributeDims.resize(attribIndex + 1, 0);
        }
        object.attributeValues[attribIndex].insert(
                object.attributeValues[attribIndex].begin(), data, data + n * dim);
        object.attributeDims[attribIndex] = dim;
    }

    glm::vec4 getAttribs(const Object &object, int attribIndex, int n, int dim) {
        Object::Buffer buf = object.attributeValues[attribIndex];
        switch (dim) {
            case 1: return glm::vec4(buf[dim*n + 0], 0, 0, 0);
            case 2: return glm::vec4(buf[dim*n + 0], buf[dim*n + 1], 0, 0);
            case 3: return glm::vec4(buf[dim*n + 0], buf[dim*n + 1], buf[dim*n + 2], 0);
            case 4: return glm::vec4(buf[dim*n + 0], buf[dim*n + 1], buf[dim*n + 2], buf[dim*n + 3]);
            default: std::cout << "That dimension is not supported!";
        }
        return glm::vec4(-1, -1, -1, -1);
    }

    // clang-format off
    template <> void Rasterizer::setVertexAttribs(Object &object, int attribIndex, int n, const float     *data) { setAttribs(object, attribIndex, n, 1, (float *)data); }
    template <> void Rasterizer::setVertexAttribs(Object &object, int attribIndex, int n, const glm::vec2 *data) { setAttribs(object, attribIndex, n, 2, (float *)data); }
    template <> void Rasterizer::setVertexAttribs(Object &object, int attribIndex, int n, const glm::vec3 *data) { setAttribs(object, attribIndex, n, 3, (float *)data); }
    template <> void Rasterizer::setVertexAttribs(Object &object, int attribIndex, int n, const glm::vec4 *data) { setAttribs(object, attribIndex, n, 4, (float *)data); }
    // clang-format on

    void Rasterizer::setTriangleIndices(Object &object, int n, glm::ivec3 *indices)
    {
        object.indices.resize(n);
        for (int i = 0; i < n; i++)
        {
            object.indices[i] = indices[i];
        }

        // printObject(object);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// RasterizerThreadPool
    ////////////////////////////////////////////////////////////////////////////

    RasterizerThreadPool::RasterizerThreadPool(size_t n_threads)
        : _n_threads(n_threads), _threads(n_threads),
          _workqueues(n_threads, std::vector<std::pair<glm::ivec2, glm::ivec2>>()), _next_workq{0}, _alive{true},
          _work{false}
    {

        _alive = true;
        for (int i = 0; i < _n_threads; i++)
        {
            // TODO CPU affinity - not so easy to set in a cross-platform manner.
            // Especially hard (impossible?) to do on MacOS w/ arm processors
            _threads[i] = std::thread([this, i] { this->_thread_fn(i); });
        }
    }

    RasterizerThreadPool::~RasterizerThreadPool()
    {
        if (_alive)
        {
            this->stop();
        }
    }

    void RasterizerThreadPool::_thread_fn(int index)
    {
        while (_alive)
        {
            /*{
                std::lock_guard<std::mutex> l(mtx);
                std::cout << "Alive" << std::endl;
            }*/
            if (!_work[index])
                continue;
            // do work
            /*{
                std::lock_guard<std::mutex> l(mtx);
                std::cout << "Working" << std::endl;
            }*/
            while (!_workqueues[index].empty())
            {
                auto [tl, br] = _workqueues[index].back();
                _fn(index, *framebuffer, *program, *z_buffer, _tri, _attrs, tl, br);
                _workqueues[index].pop_back();
            }
            // turn self off
            _work[index] = false;
        }
    }

    void RasterizerThreadPool::start()
    {
        if (!_alive)
        {
            _alive = true;
            for (int i = 0; i < _n_threads; i++)
            {
                // TODO CPU affinity - not so easy to set in a cross-platform manner.
                // Especially hard (impossible?) to do on MacOS w/ arm processors
                _threads[i] = std::thread([this, i] { this->_thread_fn(i); });
            }
        }
    }

    void RasterizerThreadPool::run()
    {
        // all of _work[i] guaranteed to be false here
        for (int i = 0; i < _n_threads; i++)
        {
            _work[i] = true;
        }

        // std::cout << "Set all threads to work. Waiting on work status." << std::endl;

        // see the work status
        bool all_free = false;
        while (!all_free)
        {
            all_free = true;
            for (int i = 0; i < _n_threads; i++)
            {
                all_free = (all_free && (!_work[i]));
            }
        }
    }

    void RasterizerThreadPool::stop()
    {
        _alive = false;
        for (int i = 0; i < _n_threads; i++)
        {
            _threads[i].join();
        }
    }

    // obviously don't call this after run has been called!
    void RasterizerThreadPool::enqueue(glm::ivec2 tl, glm::ivec2 br)
    {
        _workqueues[_next_workq].push_back({tl, br});
        _next_workq = (_next_workq + 1) % _n_threads;
    }

    void RasterizerThreadPool::set_render_function(
        std::function<void(int, SDL_Surface *, const ShaderProgram *, float *, glm::vec4 (&)[3], Attribs (&)[3],
                           glm::ivec2 &, glm::ivec2 &)>
            fn)
    {
        _fn = fn;
    }

    void RasterizerThreadPool::set_triangle_data(glm::vec4 (&tri)[3], Attribs (&attrs)[3])
    {
        _tri[0] = tri[0];
        _tri[1] = tri[1];
        _tri[2] = tri[2];
        _attrs[0] = attrs[0];
        _attrs[1] = attrs[1];
        _attrs[2] = attrs[2];
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Rendering
    ////////////////////////////////////////////////////////////////////////////

    // glm::vec3 inTriangle(glm::vec2 &sample, glm::vec2 (&triangle)[3])
    // {
    //     // idk why other methods didn't work. But this one works and seems to be faster
    //     // we can stick with this one... it is indeed faster.
    //     // the other method only checked for CCW vertex order
    //     float denominator =
    //         (triangle[0].x * (triangle[1].y - triangle[2].y) + triangle[0].y * (triangle[2].x - triangle[1].x) +
    //          triangle[1].x * triangle[2].y - triangle[1].y * triangle[2].x);
    //     float t1 = (sample.x * (triangle[2].y - triangle[0].y) + sample.y * (triangle[0].x - triangle[2].x) -
    //                 triangle[0].x * triangle[2].y + triangle[0].y * triangle[2].x) /
    //                denominator;
    //     float t2 = -(sample.x * (triangle[1].y - triangle[0].y) + sample.y * (triangle[0].x - triangle[1].x) -
    //                  triangle[0].x * triangle[1].y + triangle[0].y * triangle[1].x) /
    //                denominator;
    //     float s = 1 - t1 - t2;
    //     if (0 <= t1 && t1 <= 1 && 0 <= t2 && t2 <= 1 && t1 + t2 <= 1)
    //     {
    //         std::cout << t1 << " " << t2 << " " << 1 - t1 - t2 << "ok\n";
    //         return glm::vec3(s, t1, t2);
    //     }
    //     return glm::vec3(-1, -1, -1);
    // }

    // float dperp(glm::vec2 p, glm::vec2 p1, glm::vec2 p2)
    // {
    //     return abs((p2.x - p1.x) * (p1.y - p.y) - (p1.x - p.x) * (p2.y - p1.y)) / glm::distance(p1, p2);
    // }

    glm::vec2 sampleToPt(int x, int y, int w, int h)
    {
        return glm::vec2(-1 + 2 * (float(x) + 0.5) / w, -1 + 2 * (float(y) + 0.5) / h);
    }
    glm::ivec2 PtTosample(glm::vec2 pt, int w, int h)
    {
        return glm::ivec2(((pt.x + 1) * w) / 2 - 0.5, ((pt.y + 1) * h) / 2 - 0.5);
    }

    Uint32 vec4_to_color(SDL_PixelFormat *fmt, glm::vec4 &color)
    {
        return SDL_MapRGBA(fmt, (Uint8)(color[0] * 255), (Uint8)(color[1] * 255), (Uint8)(color[2] * 255),
                           (Uint8)(color[3] * 255));
    }

    glm::vec3 flatten(glm::vec4 hom)
    {
        return hom.xyz() / hom.w;
    }

    glm::vec3 phi(glm::vec2 (&tri)[3], glm::vec2 pt)
    {
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

    glm::vec3 phi_pc(glm::vec4 (&hom_tri)[3], glm::vec3 p, glm::vec2 pt)
    {
        float norm = (p[0] / hom_tri[0].w + p[1] / hom_tri[1].w + p[2] / hom_tri[2].w);
        return glm::vec3((p[0] / hom_tri[0].w) / norm, (p[1] / hom_tri[1].w) / norm, (p[2] / hom_tri[2].w) / norm);
    }

    glm::vec4 interpolate(glm::vec4 (&vert_attribs)[3], glm::vec3 wts)
    {
        return wts[0] * vert_attribs[0] + wts[1] * vert_attribs[1] + wts[2] * vert_attribs[2];
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Rasterizer methods
    ////////////////////////////////////////////////////////////////////////////

    void Rasterizer::enableDepthTest()
    {
        depth_enabled = true;
        z_buffer = new float[framebuffer->w * framebuffer->h];
        for (int i = 0; i < framebuffer->w; i++)
        {
            for (int j = 0; j < framebuffer->h; j++)
            {
                z_buffer[framebuffer->h * i + j] = 1;
            }
        }
    }

    void Rasterizer::clear(glm::vec4 color)
    {
        SDL_FillRect(framebuffer, NULL, vec4_to_color(framebuffer->format, color));
        if (depth_enabled)
        {
            for (int i = 0; i < framebuffer->w; i++)
            {
                for (int j = 0; j < framebuffer->h; j++)
                {
                    z_buffer[framebuffer->h * i + j] = 1;
                }
            }
        }
    }

#define pix2pt(x, y, p) glm::vec2((2 * (x) + 1) * p[0] - 1, (2 * (y) + 1) * p[1] - 1)
#define pt2pix(x, y, p) glm::ivec2(round(((x) + 1) / (2 * p[0]) - 0.5f), round(((y) + 1) / (2 * p[1]) - 0.5f))

    void rasterize_block(int idx,                                             // thread index
                         SDL_Surface *fb, const ShaderProgram *sp, float *zb, // common buffers to write to
                         glm::vec4 (&hom_tri)[3], Attribs (&attrs)[3],        // triangle-specific attributes
                         glm::ivec2 &tl, glm::ivec2 &br                       // top-left and bottom-right pixel bounds
    )
    {

        // std::cout << "In rasterize_block" << std::endl;
        Uint32 *pixels = (Uint32 *)fb->pixels;
        SDL_PixelFormat *format = fb->format;
        int h = fb->h;
        int w = fb->w;
        glm::vec2 p(1.0f / w, 1.0f / h);
        glm::vec2 tri[3] = {flatten(hom_tri[0]).xy(), flatten(hom_tri[1]).xy(), flatten(hom_tri[2]).xy()};

        // square ignore test:
        //    for all edges of the triangle
        //        check if the other point of the triangle is on the opposite side
        //        compared to all four points of the square

        glm::vec2 p1 = pix2pt(tl.x, tl.y, p);
        glm::vec2 p2 = pix2pt(tl.x, br.y, p);
        glm::vec2 p3 = pix2pt(br.x, br.y, p);
        glm::vec2 p4 = pix2pt(br.x, tl.y, p);

        // std::cout << "Rasterizing" << std::endl;

        for (int k = 0; k < 3; k++)
        {
            auto v1 = tri[k % 3];
            auto v2 = tri[(k + 1) % 3];
            auto v3 = tri[(k + 2) % 3];

            auto s = v2 - v1;
            glm::vec2 n(-s.y, s.x);
            float t = glm::dot(n, v3 - v1);

            float d1 = glm::dot((p1 - v1), n);
            float d2 = glm::dot((p2 - v1), n);
            float d3 = glm::dot((p3 - v1), n);
            float d4 = glm::dot((p4 - v1), n);

            if (d1 * t < 0 && d2 * t < 0 && d3 * t < 0 && d4 * t < 0)
            {
                // tri outside sq
                return;
            }
        }

        for (int y = std::max(0, tl.y); y <= std::min(h - 1, br.y); y++)
        {
            for (int x = std::max(0, tl.x); x <= std::min(w - 1, br.x); x++)
            {
                glm::vec2 px = pix2pt(x, y, p);
                glm::vec3 p = phi(tri, px);
                glm::vec3 p_pc = phi_pc(hom_tri, p, px);

                if (p[0] < 0 || p[1] < 0 || p[2] < 0)
                    continue;

                if (zb != nullptr)
                {
                    float z = hom_tri[0].z * p_pc[0] / hom_tri[0].w + hom_tri[1].z * p_pc[1] / hom_tri[1].w +
                              hom_tri[2].z * p_pc[2] / hom_tri[2].w;
                    if (z > zb[(h - y - 1) * w + x])
                    {
                        continue; // discard fragment
                    }
                    zb[(h - y - 1) * w + x] = z;
                }

                // load and interpolate attributes
                Attribs interp_attrs;
                for (int i = 0; i < attrs[0].size(); i++)
                {
                    // assuming vec4 here.
                    glm::vec4 vert_attribs[3] = {
                        attrs[0].get<glm::vec4>(i),
                        attrs[1].get<glm::vec4>(i),
                        attrs[2].get<glm::vec4>(i),
                    };
                    interp_attrs.set<glm::vec4>(i, interpolate(vert_attribs, p_pc));
                }

                glm::vec4 color = sp->fs(sp->uniforms, interp_attrs);
                pixels[(h - y - 1) * w + x] = vec4_to_color(format, color);
            }
        }
    }

    // Draws the triangles of the given object.
    void Rasterizer::drawObject(const Object &object)
    {
        // not sure how slow/fast spawning threads is, but the alternative is
        // to sleep them. Let's see.
        // rtp->start();

        Uint32 *pixels = (Uint32 *)framebuffer->pixels;
        SDL_PixelFormat *format = framebuffer->format;
        int h = framebuffer->h;
        int w = framebuffer->w;
        glm::vec2 p(1.0f / w, 1.0f / h);

        int vertex_count = object.attributeValues[0].size() / object.attributeDims[0];
        std::vector<Attribs> vertex_in_attrs(vertex_count);
        std::vector<Attribs> vertex_out_attrs(vertex_count);
        std::vector<glm::vec4> vertex_pos(vertex_count);

        // vertex shaders
        for (int v = 0; v < vertex_count; v++)
        {
            for (int i = 0; i < object.attributeValues.size(); i++)
            {
                vertex_in_attrs[v].set<glm::vec4>(i, getAttribs(object, i, v, object.attributeDims[i]));
            }
            vertex_pos[v] = shader_program->vs(shader_program->uniforms, vertex_in_attrs[v], vertex_out_attrs[v]);
        }

        rtp->set_render_function(rasterize_block);

        auto render_triangle = [&](glm::ivec3 &idxs) {
            glm::vec4 hom_tri[3] = {vertex_pos[idxs[0]], vertex_pos[idxs[1]], vertex_pos[idxs[2]]};
            glm::vec3 tri[3] = {flatten(hom_tri[0]), flatten(hom_tri[1]), flatten(hom_tri[2])};

            float xmin = fminf(tri[0].x, fminf(tri[1].x, tri[2].x)), ymin = fminf(tri[0].y, fminf(tri[1].y, tri[2].y));
            float xmax = fmaxf(tri[0].x, fmaxf(tri[1].x, tri[2].x)), ymax = fmaxf(tri[0].y, fmaxf(tri[1].y, tri[2].y));

            glm::ivec2 tl = pt2pix(xmin - p.x, ymin - p.y, p);
            glm::ivec2 br = pt2pix(xmax + p.x, ymax + p.y, p);

            int cw = 16, ch = 16; // divisible by total dim

            Attribs attrs[3] = {vertex_out_attrs[idxs[0]], vertex_out_attrs[idxs[1]], vertex_out_attrs[idxs[2]]};

            rtp->set_triangle_data(hom_tri, attrs);

            // chunk this up and enqueue
            for (int i = (tl.y / ch) * ch; i <= (br.y / ch) * ch; i += ch)
            {
                for (int j = (tl.x / cw) * cw; j <= (br.x / cw) * cw; j += cw)
                {
                    rtp->enqueue(glm::ivec2(j, i), glm::ivec2(j + cw - 1, i + cw - 1));
                }
            }
            // std::cout << "Starting rendering via multithreading" << std::endl;
            rtp->run();
        };

        // render_triangle(object.indices[0]);
        for (glm::ivec3 idxs : object.indices)
        {
            render_triangle(idxs);
        }

        // rtp->stop();
    }

    // Displays the framebuffer on the screen.
    // This can probably be optimized/parallelized but it works for now
    // Use a dirty bit buffer to mark bits that have been updated?
    void Rasterizer::show()
    {
        auto windowSurface = SDL_GetWindowSurface(window);
        int w = framebuffer->w;
        int b = w / windowSurface->w;
        if (b == 1)
        {
            SDL_BlitSurface(framebuffer, NULL, windowSurface, NULL);
            SDL_UpdateWindowSurface(window);
            return;
        }
        int buf, avg[3];
        Uint8 px[3];
        for (int i = 0; i < windowSurface->h; i++)
        {
            for (int j = 0; j < windowSurface->w; j++)
            {
                avg[0] = avg[1] = avg[2] = 0;
                for (int k = 0; k < b; k++)
                {
                    for (int l = 0; l < b; l++)
                    {
                        buf = ((Uint32 *)framebuffer->pixels)[w * (i * b + k) + j * b + l];
                        SDL_GetRGB(buf, framebuffer->format, px, (px + 1), (px + 2));
                        avg[0] += px[0];
                        avg[1] += px[1];
                        avg[2] += px[2];
                    }
                }
                avg[0] /= b * b;
                avg[1] /= b * b;
                avg[2] /= b * b;
                ((Uint32 *)windowSurface->pixels)[windowSurface->w * i + j] =
                    SDL_MapRGB(windowSurface->format, avg[0], avg[1], avg[2]);
            }
        }
        // SDL_BlitScaled(framebuffer, NULL, windowSurface, NULL);
        SDL_UpdateWindowSurface(window);
    }

} // namespace Software
} // namespace COL781
