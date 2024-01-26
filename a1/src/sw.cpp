#include "SDL2/SDL_pixels.h"
#define GLM_SWIZZLE
#include "sw.hpp"

#include <iostream>
#include <vector>

namespace COL781
{
namespace Software
{

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

        printObject(object);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Rendering
    ////////////////////////////////////////////////////////////////////////////

    bool inTriangle(glm::vec2 &sample, glm::vec2 (&triangle)[3])
    {
        for (int k = 0; k < 3; k++)
        {
            glm::vec2 v1 = triangle[k % 3];
            glm::vec2 v2 = triangle[(k + 1) % 3];
            glm::vec2 v3 = triangle[(k + 2) % 3];
            glm::vec2 s = v2 - v1;
            glm::vec2 t(-s.y, s.x);

            float dot = glm::dot(t, sample - v1);
            float vdot = glm::dot(t, v3 - v1);
            if (dot * vdot < 0)
            {
                return false;
            }
        }
        return true;
    }

    float dperp(glm::vec2 p, glm::vec2 p1, glm::vec2 p2)
    {
        return abs((p2.x - p1.x) * (p1.y - p.y) - (p1.x - p.x) * (p2.y - p1.y)) / glm::distance(p1, p2);
    }

    glm::vec2 sampleToPt(int x, int y, int w, int h)
    {
        return glm::vec2(-1 + 2 * float(x) / w, -1 + 2 * float(y) / h);
    }

    Uint32 vec4_to_color(SDL_PixelFormat *fmt, glm::vec4 color)
    {
        return SDL_MapRGBA(fmt, (Uint8)(color[0] * 255), (Uint8)(color[1] * 255), (Uint8)(color[2] * 255),
                           (Uint8)(color[3] * 255));
    }

    glm::vec3 flatten(glm::vec4 hom) {
        return hom.xyz()/hom.w;
    }

    glm::vec3 phi(glm::vec2 (&tri)[3], glm::vec2 pt) {
        float norm = dperp(tri[0], tri[1], tri[2]);
        return glm::vec3(
                dperp(pt, tri[1], tri[2]) / dperp(tri[0], tri[1], tri[2]),
                dperp(pt, tri[2], tri[0]) / dperp(tri[1], tri[2], tri[0]),
                dperp(pt, tri[0], tri[1]) / dperp(tri[2], tri[0], tri[1]) 
            );
    }

    glm::vec3 phi_pc(glm::vec4 (&hom_tri)[3], glm::vec3 p, glm::vec2 pt) {
        float norm = (p[0]/hom_tri[0].w + p[1]/hom_tri[1].w + p[2]/hom_tri[2].w);
        return glm::vec3(
                (p[0] / hom_tri[0].w) / norm,
                (p[1] / hom_tri[1].w) / norm,
                (p[2] / hom_tri[2].w) / norm
            );
    }

    glm::vec4 interpolate(glm::vec4 (&vert_attribs)[3], glm::vec3 wts) {
        return wts[0]*vert_attribs[0] + wts[1]*vert_attribs[1] + wts[2]*vert_attribs[2];
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Rasterizer methods
    ////////////////////////////////////////////////////////////////////////////

    void Rasterizer::enableDepthTest()
    {
        depth_enabled = true;
        z_buffer = new float[framebuffer->w * framebuffer->h];
        for (int i=0; i<framebuffer->w; i++) {
            for (int j=0; j<framebuffer->h; j++) {
                z_buffer[framebuffer->h*i + j] = 0;
            }
        }
    }

    void Rasterizer::clear(glm::vec4 color)
    {
        SDL_FillRect(framebuffer, NULL, vec4_to_color(framebuffer->format, color));
        if (depth_enabled) {
            for (int i=0; i<framebuffer->w; i++) {
                for (int j=0; j<framebuffer->h; j++) {
                    z_buffer[framebuffer->h*i + j] = 0;
                }
            }
        }
    }

    // Draws the triangles of the given object.
    void Rasterizer::drawObject(const Object &object)
    {
        Uint32 *pixels = (Uint32 *)framebuffer->pixels;
        SDL_PixelFormat *format = framebuffer->format;
        int h = framebuffer->h;
        int w = framebuffer->w;

        int vertex_count = object.attributeValues[0].size() / object.attributeDims[0];
        std::vector<Attribs> vertex_in_attrs(vertex_count);
        std::vector<Attribs> vertex_out_attrs(vertex_count);
        std::vector<glm::vec4> vertex_pos(vertex_count);

        // vertex shaders
        for (int v = 0; v < vertex_count; v++)
        {
            for (int i = 0; i < object.attributeValues.size(); i++)
            {
                int dim = object.attributeDims[i];
                vertex_in_attrs[v].set<glm::vec4>(i, getAttribs(object, i, v, dim));
            }
            vertex_pos[v] = shader_program->vs(
                    shader_program->uniforms, vertex_in_attrs[v], vertex_out_attrs[v]
                );
        }

        auto render_triangle = [&](glm::ivec3 idxs) {
            glm::vec4 hom_tri[3] = { 
                vertex_pos[idxs[0]], 
                vertex_pos[idxs[1]], 
                vertex_pos[idxs[2]] 
            };
            glm::vec2 tri[3] = {
                flatten(hom_tri[0]).xy(), 
                flatten(hom_tri[1]).xy(), 
                flatten(hom_tri[2]).xy()
            };

            /*
            std::cout << tri[0].x << "," << tri[0].y << " " 
                      << tri[1].x << "," << tri[1].y << " " 
                      << tri[2].x << "," << tri[2].y << std::endl;
            */

            std::vector<std::tuple<int,int,glm::vec2>> fragments;

            // TODO make this scanline
            for (int x = 0; x < w; x++)
            {
                for (int y = 0; y < h; y++)
                {
                    glm::vec2 pt = sampleToPt(x, y, w, h);
                    if (inTriangle(pt, tri)) fragments.push_back({x, y, pt});
                }
            }

            for (auto [x, y, pt] : fragments) {
                glm::vec3 p = phi(tri, pt);
                glm::vec3 p_pc = phi_pc(hom_tri, p, pt);
                float z = hom_tri[0].w*p_pc[0]/hom_tri[0].z + hom_tri[1].w*p_pc[1]/hom_tri[1].z + hom_tri[2].w*p_pc[2]/hom_tri[2].z;
                if (z < z_buffer[(h-y-1)*w + x]) {
                    continue; // discard fragment
                }
                z_buffer[(h-y-1)*w + x] = z;
                // load and interpolate attributes
                Attribs interp_attrs;
                // HACK: find a way to iterate over the attributes present in 
                // out_attrs, instead of just guessing.
                for (int i=0; i<object.attributeValues.size()-1; i++) {
                    glm::vec4 vert_attribs[3] = {
                        vertex_out_attrs[idxs[0]].get<glm::vec4>(i),
                        vertex_out_attrs[idxs[1]].get<glm::vec4>(i),
                        vertex_out_attrs[idxs[2]].get<glm::vec4>(i),
                    };
                    interp_attrs.set<glm::vec4>(i, interpolate(vert_attribs, p_pc));
                }

                glm::vec4 color = shader_program->fs(shader_program->uniforms, interp_attrs);
                pixels[(h-y-1)*w + x] = vec4_to_color(format, color);
            }
        };

        for (glm::ivec3 idxs : object.indices)
        {
            render_triangle(idxs);
        }
    }

    // Displays the framebuffer on the screen.
    // This can probably be optimized/parallelized but eh, it works for now
    void Rasterizer::show()
    {
        auto windowSurface = SDL_GetWindowSurface(window);
        int w = framebuffer->w;
        int b = w/windowSurface->w;
        int buf, avg[3];
        Uint8 px[3];
        for (int i=0; i<windowSurface->h; i++) {
            for (int j=0; j<windowSurface->w; j++) {
                avg[0] = avg[1] = avg[2] = 0;
                for (int k=0; k<b; k++) {
                    for (int l=0; l<b; l++) {
                        buf = ((Uint32*)framebuffer->pixels)[w*(i*b + k) + j*b + l];
                        SDL_GetRGB(buf, framebuffer->format, px, (px+1), (px+2));
                        avg[0] += px[0];
                        avg[1] += px[1];
                        avg[2] += px[2];
                    }
                }
                avg[0] /= b*b;
                avg[1] /= b*b;
                avg[2] /= b*b;
                ((Uint32*)windowSurface->pixels)[windowSurface->w*i + j] = SDL_MapRGB(windowSurface->format, avg[0], avg[1], avg[2]);
            }
        }
        // SDL_BlitScaled(framebuffer, NULL, windowSurface, NULL);
        SDL_UpdateWindowSurface(window);
    }

} // namespace Software
} // namespace COL781
