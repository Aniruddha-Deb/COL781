#ifndef SW_HPP
#define SW_HPP

#include <glm/glm.hpp>
#include <map>
#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <thread>
#include <functional>

namespace COL781
{
namespace Software
{

    class Attribs
    {
        // A class to contain the attributes of ONE vertex
      public:
        // only float, glm::vec2, glm::vec3, glm::vec4 allowed
        template <typename T> T get(int attribIndex) const;
        template <typename T> void set(int attribIndex, T value);
        int size();

      private:
        std::vector<glm::vec4> values;
        std::vector<int> dims;
    };

    class Uniforms
    {
        // A class to contain all the uniform variables
      public:
        // any type allowed
        template <typename T> T get(const std::string &name) const
        {
            return *(T *)values.at(name);
        }

        template <typename T> void set(const std::string &name, T value)
        {
            auto it = values.find(name);
            if (it != values.end())
            {
                delete it->second;
            }
            values[name] = (void *)(new T(value));
        }

      private:
        std::map<std::string, void *> values;
    };

    /* A vertex shader is a function that:
       reads the uniform variables and one vertex's input attributes,
       writes the output attributes for interpolation to fragments,
       and returns the vertex position in NDC as a homogeneous vec4. */
    using VertexShader = glm::vec4 (*)(const Uniforms &uniforms, const Attribs &in, Attribs &out);

    /* A fragment shader is a function that:
       reads the uniform variables and one fragment's interpolated attributes,
       and returns the colour of the fragment as an RGBA value. */
    using FragmentShader = glm::vec4 (*)(const Uniforms &uniforms, const Attribs &in);

    struct ShaderProgram
    {
        VertexShader vs;
        FragmentShader fs;
        Uniforms uniforms;
    };

    struct Object
    {
        using Buffer = std::vector<float>;
        std::vector<Buffer> attributeValues;
        std::vector<int> attributeDims;
        std::vector<glm::ivec3> indices;
    };

    class RasterizerThreadPool;

    class Rasterizer {
        public:
#include "api.inc"
        private:
            SDL_Window* window;
            bool quit;

            int spp = 1;
            SDL_Surface *framebuffer;
            const ShaderProgram* shader_program;

            RasterizerThreadPool *rtp;

            bool depth_enabled = false;
            float *z_buffer;
    };

    class RasterizerThreadPool
    {
      public:
        RasterizerThreadPool(size_t n_threads);
        ~RasterizerThreadPool();
        void run();
        void start();
        void stop();
        void enqueue(glm::ivec2 tl, glm::ivec2 br);
        void set_render_function(
            std::function<void(int,                                           // thread index
                               SDL_Surface *, const ShaderProgram *, float *, // common buffers to write to
                               glm::vec4 (&)[3], Attribs (&)[3],              // triangle-specific attributes
                               glm::ivec2 &, glm::ivec2 &                     // top-left and bottom-right pixel bounds
                               )>
                fn);
        void set_triangle_data(glm::vec4 (&tri)[3], Attribs (&attrs)[3]);

        SDL_Surface **framebuffer;
        const ShaderProgram **program;
        float **z_buffer;

      private:
        void _thread_fn(int thread_idx);

        size_t _n_threads;
        std::vector<std::thread> _threads;
        std::vector<std::vector<std::pair<glm::ivec2, glm::ivec2>>> _workqueues;
        int _next_workq;
        volatile bool _alive;
        volatile bool _work[12]; // capping _work size but should be ok
        std::function<void(int, SDL_Surface *, const ShaderProgram *, float *, glm::vec4 (&)[3], Attribs (&)[3],
                           glm::ivec2 &, glm::ivec2 &)>
            _fn;

        glm::vec4 _tri[3];
        Attribs _attrs[3];
    };

} // namespace Software
} // namespace COL781

#endif
