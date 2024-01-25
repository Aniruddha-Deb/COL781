#include "SDL2/SDL_pixels.h"
#define GLM_SWIZZLE
#include "sw.hpp"

#include <iostream>
#include <vector>

#define RED_MASK   0x0000FF
#define GREEN_MASK 0x00FF00
#define BLUE_MASK  0xFF0000

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

    bool ptInTriangle(glm::vec3 (&triangle)[3], glm::vec2 &pt)
    {
        for (int k = 0; k < 3; k++)
        {
            glm::vec2 v1 = triangle[k % 3].xy();
            glm::vec2 v2 = triangle[(k + 1) % 3].xy();
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

    glm::vec2 pix_to_pt(int x, int y, int w, int h)
    {
        return glm::vec2(-1 + 2 * float(x) / w, -1 + 2 * float(y) / h);
    }

    Uint32 vec4_to_color(SDL_PixelFormat *fmt, glm::vec4 color)
    {
        return SDL_MapRGBA(fmt, (Uint8)(color[0] * 255), (Uint8)(color[1] * 255), (Uint8)(color[2] * 255),
                           (Uint8)(color[3] * 255));
    }

    // Clear the framebuffer, setting all pixels to the given color.
    float distance_point_to_line(glm::vec2 p, glm::vec2 p1, glm::vec2 p2)
    {
        return abs((p2.x - p1.x) * (p1.y - p.y) - (p1.x - p.x) * (p2.y - p1.y)) / glm::distance(p1, p2);
    }
    // glm::vec2 operator/(const glm::vec2 &lhs, const glm::vec2 &rhs)
    // {
    //     return glm::vec2(lhs.x / rhs.x, lhs.y / rhs.y);
    // }

    // glm::vec3 operator/(const glm::vec3 &lhs, const glm::vec3 &rhs)
    // {
    //     return glm::vec3(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z);
    // }

    // glm::vec4(const glm::vec4 &lhs, const glm::vec4 &rhs)
    // {
    //     return glm::vec4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w);
    // }
    glm::vec4 interpolate(glm::vec3 (&triangle)[3], glm::vec2 pix, std::vector<glm::vec4> attribute, bool depth_enabled)
    {
        if (!depth_enabled)
        {
            float phi1 = distance_point_to_line(pix, triangle[1], triangle[2]) /
                         distance_point_to_line(triangle[0], triangle[1], triangle[2]);
            float phi2 = distance_point_to_line(pix, triangle[0], triangle[2]) /
                         distance_point_to_line(triangle[1], triangle[0], triangle[2]);
            float phi3 = distance_point_to_line(pix, triangle[0], triangle[1]) /
                         distance_point_to_line(triangle[2], triangle[0], triangle[1]);
            return (phi1 * attribute[0] + phi2 * attribute[1] + phi3 * attribute[2]);
        }
        else
        {
            std::cout << triangle[0].z << " " << triangle[1].z << " " << triangle[2].z << std::endl;
            float phi1 = distance_point_to_line(pix, triangle[1], triangle[2]) /
                         distance_point_to_line(triangle[0], triangle[1], triangle[2]);
            float phi2 = distance_point_to_line(pix, triangle[0], triangle[2]) /
                         distance_point_to_line(triangle[1], triangle[0], triangle[2]);
            float phi3 = distance_point_to_line(pix, triangle[0], triangle[1]) /
                         distance_point_to_line(triangle[2], triangle[0], triangle[1]);
            // std::cout << ((phi1 * attribute[0] / triangle[0].z + phi2 * attribute[1] / triangle[1].z +
            //                phi3 * attribute[2] / triangle[2].z) /
            //               (phi1 / triangle[0].z + phi2 / triangle[1].z + phi3 / triangle[2].z))[0]
            //           << std::endl;
            return (phi1 * attribute[0] / triangle[0].z + phi2 * attribute[1] / triangle[1].z +
                    phi3 * attribute[2] / triangle[2].z) /
                   (phi1 / triangle[0].z + phi2 / triangle[1].z + phi3 / triangle[2].z);
        }
    }

    void render_naive(glm::ivec3 idx, std::vector<Attribs> &vertex_attribs, std::vector<glm::vec4> &vertex_pos,
                      const Uniforms &uniforms, int attribute_cnt, FragmentShader fs, SDL_Surface *framebuffer, int spp,
                      bool depth_enabled)
    {
        // std::cout << vertex_pos[idx.x].x << " " << vertex_pos[idx.x].y << " " << vertex_pos[idx.x].z << " "
        //           << vertex_pos[idx.x].w << "\n";
        glm::vec3 triangle[3] = {glm::vec3(vertex_pos[idx.x].x, vertex_pos[idx.x].y, vertex_pos[idx.x].z),
                                 glm::vec3(vertex_pos[idx.y].x, vertex_pos[idx.y].y, vertex_pos[idx.x].z),
                                 glm::vec3(vertex_pos[idx.z].x, vertex_pos[idx.z].y, vertex_pos[idx.x].z)};
        Uint32 *pixels = (Uint32 *)framebuffer->pixels;
        SDL_PixelFormat *format = framebuffer->format;
        int h = framebuffer->h;
        int w = framebuffer->w;
        float pw = 2.f / w;
        for (int i = 0; i < w; i++)
        {
            for (int j = 0; j < h; j++)
            {
                Uint32 background = pixels[(h - j - 1) * w + i];
                Uint32 foreground;
                // glm::vec4 fragment_coords = interpolate(triangle, pix_tl, vertex_pos);
                glm::vec2 pix_tl = pix_to_pt(i, j, w, h);
                // float v = value_supersample(triangle, pix_tl, pw, spp);
                if (ptInTriangle(triangle, pix_tl))
                {
                    Attribs fragment_attributes;
                    for (int attribute = 0; attribute < attribute_cnt; attribute++)
                    {
                        fragment_attributes.set(attribute,
                                                interpolate(triangle, pix_tl,
                                                            {vertex_attribs[idx.x].get<glm::vec4>(attribute),
                                                             vertex_attribs[idx.y].get<glm::vec4>(attribute),
                                                             vertex_attribs[idx.z].get<glm::vec4>(attribute)},
                                                            depth_enabled));
                    }
                    foreground = vec4_to_color(format, fs(uniforms, fragment_attributes));
                    // if (depth_enabled && z_buffer[(h-j-1)*w + i] > z ) {

                    pixels[(h - j - 1) * w + i] = foreground;
                    // }
                }
            }
        }
    }

    glm::vec4 GetAttribs(const std::vector<std::vector<float>> &attributeValues, int attrib_idx, int vertex_idx)
    {
        return glm::vec4(attributeValues[attrib_idx][4 * vertex_idx], attributeValues[attrib_idx][4 * vertex_idx + 1],
                         attributeValues[attrib_idx][4 * vertex_idx + 2],
                         attributeValues[attrib_idx][4 * vertex_idx + 3]);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Rasterizer methods
    ////////////////////////////////////////////////////////////////////////////

    void Rasterizer::enableDepthTest()
    {
        depth_enabled = true;
        z_buffer = new Uint16[framebuffer->w * framebuffer->h];
    }

    void Rasterizer::clear(glm::vec4 color)
    {
        SDL_FillRect(framebuffer, NULL, vec4_to_color(framebuffer->format, color));
    }

    // Draws the triangles of the given object.
    void Rasterizer::drawObject(const Object &object)
    {
        // Implement the rendering pipeline here
        int vertex_count = object.attributeValues[0].size() / object.attributeDims[0];
        std::vector<Attribs> vertex_attribs(vertex_count);
        std::vector<glm::vec4> vertex_pos(vertex_count);
        for (int vertex = 0; vertex < vertex_count; vertex++)
        {
            for (int attribute = 0; attribute < object.attributeValues.size(); attribute++)
            {
                vertex_attribs[vertex].set(attribute, GetAttribs(object.attributeValues, attribute, vertex));
            }
            vertex_pos[vertex] =
                shader_program->vs(shader_program->uniforms, vertex_attribs[vertex], vertex_attribs[vertex]);
        }
        for (glm::ivec3 idx : object.indices)
        {
            render_naive(idx, vertex_attribs, vertex_pos, shader_program->uniforms, object.attributeValues.size() - 1,
                         shader_program->fs, framebuffer, spp, depth_enabled);
        }
    }

    // Displays the framebuffer on the screen.
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
