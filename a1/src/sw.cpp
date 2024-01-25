#define GLM_SWIZZLE
#include "sw.hpp"

#include <iostream>
#include <vector>

namespace COL781
{
namespace Software
{

    // Forward declarations

    template <> float Attribs::get(int index) const;
    template <> glm::vec2 Attribs::get(int index) const;
    template <> glm::vec3 Attribs::get(int index) const;
    template <> glm::vec4 Attribs::get(int index) const;

    template <> void Attribs::set(int index, float value);
    template <> void Attribs::set(int index, glm::vec2 value);
    template <> void Attribs::set(int index, glm::vec3 value);
    template <> void Attribs::set(int index, glm::vec4 value);

    // Built-in shaders

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

    // Implementation of Attribs and Uniforms classes

    void checkDimension(int index, int actual, int requested)
    {
        if (actual != requested)
        {
            std::cout << "Warning: attribute " << index << " has dimension " << actual << " but accessed as dimension "
                      << requested << std::endl;
        }
    }

    template <> float Attribs::get(int index) const
    {
        checkDimension(index, dims[index], 1);
        return values[index].x;
    }

    template <> glm::vec2 Attribs::get(int index) const
    {
        checkDimension(index, dims[index], 2);
        return glm::vec2(values[index].x, values[index].y);
    }

    template <> glm::vec3 Attribs::get(int index) const
    {
        checkDimension(index, dims[index], 3);
        return glm::vec3(values[index].x, values[index].y, values[index].z);
    }

    template <> glm::vec4 Attribs::get(int index) const
    {
        checkDimension(index, dims[index], 4);
        return values[index];
    }

    void expand(std::vector<int> &dims, std::vector<glm::vec4> &values, int index)
    {
        if (dims.size() < index + 1)
            dims.resize(index + 1);
        if (values.size() < index + 1)
            values.resize(index + 1);
    }

    template <> void Attribs::set(int index, float value)
    {
        expand(dims, values, index);
        dims[index] = 1;
        values[index].x = value;
    }

    template <> void Attribs::set(int index, glm::vec2 value)
    {
        expand(dims, values, index);
        dims[index] = 2;
        values[index].x = value.x;
        values[index].y = value.y;
    }

    template <> void Attribs::set(int index, glm::vec3 value)
    {
        expand(dims, values, index);
        dims[index] = 3;
        values[index].x = value.x;
        values[index].y = value.y;
        values[index].z = value.z;
    }

    template <> void Attribs::set(int index, glm::vec4 value)
    {
        expand(dims, values, index);
        dims[index] = 4;
        values[index] = value;
    }

    // Creates a window with the given title, size, and samples per pixel.
    bool Rasterizer::initialize(const std::string &title, int width, int height, int _spp)
    {
        bool success = true;
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        {
            printf("SDL could not initialize! SDL_Error: %s", SDL_GetError());
            success = false;
        }
        else
        {
            int screenWidth = width;
            int screenHeight = height; // do we multiply with spp here? I think spp is for aliasing.
            spp = _spp;
            window = SDL_CreateWindow("COL781", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth,
                                      screenHeight, SDL_WINDOW_SHOWN);
            if (window == NULL)
            {
                printf("Window could not be created! SDL_Error: %s", SDL_GetError());
                success = false;
            }
            else
            {
                // with an alpha channel this time
                framebuffer =
                    SDL_CreateRGBSurface(0, width, height, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
            }
        }
        return success;
    }

    // Returns true if the user has requested to quit the program.
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

    /** Shader programs **/

    // Creates a new shader program, i.e. a pair of a vertex shader and a fragment shader.
    ShaderProgram Rasterizer::createShaderProgram(const VertexShader &vs, const FragmentShader &fs)
    {
        ShaderProgram sp;
        sp.vs = vs;
        sp.fs = fs;
        sp.uniforms = Uniforms();
        return sp;
    }

    // Makes the given shader program active. Future draw calls will use its vertex and fragment shaders.
    void Rasterizer::useShaderProgram(const ShaderProgram &program)
    {
        shader_program = &program;
    }

    // Sets the value of a uniform variable.
    // T is only allowed to be float, int, glm::vec2/3/4, glm::mat2/3/4.
    template <typename T> void Rasterizer::setUniform(ShaderProgram &program, const std::string &name, T value)
    {
        program.uniforms.set(name, value);
    }

    // Deletes the given shader program.
    void Rasterizer::deleteShaderProgram(ShaderProgram &program)
    {
        if (&program == shader_program)
        {
            shader_program = nullptr;
        }
    }

    /** Objects **/

    // Creates an object, i.e. a collection of vertices and triangles.
    // Vertex attribute arrays store the vertex data.
    // A triangle index array stores the indices of the triangles.
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
        obj.attributeValues = std::vector<Object::Buffer>();
        obj.attributeDims = std::vector<int>();
        obj.indices = std::vector<glm::ivec3>();
        return obj;
    }

    void printObject(const Object &obj)
    {
        // Print the number of attribute buffers
        std::cout << "Number of attribute buffers: " << obj.attributeValues.size() << "\n";

        // Iterate over each attribute buffer and print its dimensions and values
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

        // Print the number of indices
        std::cout << "Number of indices: " << obj.indices.size() << "\n";

        // Iterate over each index and print its value
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
        object.attributeValues[attribIndex].insert(object.attributeValues[attribIndex].begin(), data, data + n * dim);
        object.attributeDims[attribIndex] = dim;
    }

    template <> void Rasterizer::setVertexAttribs(Object &object, int attribIndex, int n, const float *data)
    {
        setAttribs(object, attribIndex, n, 1, data);
    }

    template <> void Rasterizer::setVertexAttribs(Object &object, int attribIndex, int n, const glm::vec2 *data)
    {
        setAttribs(object, attribIndex, n, 2, (float *)data);
    }

    template <> void Rasterizer::setVertexAttribs(Object &object, int attribIndex, int n, const glm::vec3 *data)
    {
        setAttribs(object, attribIndex, n, 3, (float *)data);
    }

    template <> void Rasterizer::setVertexAttribs(Object &object, int attribIndex, int n, const glm::vec4 *data)
    {
        setAttribs(object, attribIndex, n, 4, (float *)data);
    }

    template <> void Rasterizer::setUniform(ShaderProgram &sp, const std::string &name, float value)
    {
        sp.uniforms.set<float>(name, value);
    }

    template <> void Rasterizer::setUniform(ShaderProgram &sp, const std::string &name, int value)
    {
        sp.uniforms.set<int>(name, value);
    }

    template <> void Rasterizer::setUniform(ShaderProgram &sp, const std::string &name, glm::vec2 value)
    {
        sp.uniforms.set<glm::vec2>(name, value);
    }

    template <> void Rasterizer::setUniform(ShaderProgram &sp, const std::string &name, glm::mat2 value)
    {
        sp.uniforms.set<glm::mat2>(name, value);
    }

    template <> void Rasterizer::setUniform(ShaderProgram &sp, const std::string &name, glm::vec3 value)
    {
        sp.uniforms.set<glm::vec3>(name, value);
    }

    template <> void Rasterizer::setUniform(ShaderProgram &sp, const std::string &name, glm::mat3 value)
    {
        sp.uniforms.set<glm::mat3>(name, value);
    }

    template <> void Rasterizer::setUniform(ShaderProgram &sp, const std::string &name, glm::vec4 value)
    {
        sp.uniforms.set<glm::vec4>(name, value);
    }

    template <> void Rasterizer::setUniform(ShaderProgram &sp, const std::string &name, glm::mat4 value)
    {
        sp.uniforms.set<glm::mat4>(name, value);
    }

    // Sets the indices of the triangles.
    void Rasterizer::setTriangleIndices(Object &object, int n, glm::ivec3 *indices)
    {
        object.indices.resize(n);
        for (int i = 0; i < n; i++)
        {
            object.indices[i] = indices[i];
        }

        printObject(object);
    }

    /** Drawing **/

    bool membership_check(glm::vec3 (&triangle)[3], glm::vec2 &pt)
    {
        // std::cout << triangle[0].x << " " << triangle[0].y << std::endl;
        // std::cout << triangle[1].x << " " << triangle[1].y << std::endl;
        // std::cout << triangle[2].x << " " << triangle[2].y << std::endl;
        for (int k = 0; k < 3; k++)
        {
            glm::vec2 v1 = triangle[k % 3].xy();
            glm::vec2 v2 = triangle[(k + 1) % 3].xy();
            glm::vec2 s = v2 - v1;
            glm::vec2 t(-s.y, s.x);

            float dot = glm::dot(t, pt - v1);
            // std::cout << dot << std::endl;
            if (dot < 0)
            {
                return false;
            }
        }
        return true;
    }

    Uint32 pix_blend(Uint32 new_pix, Uint32 old_pix, SDL_PixelFormat *format)
    {
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

    float value_supersample(glm::vec3 (&triangle)[3], glm::vec3 pix, float pix_w, int spp)
    {
        int samples_inside = 0;
        int w = sqrt(spp);
        int pitch = pix_w / (w + 1);
        auto tl_sample = pix.xy() + glm::vec2(pitch / 2, pitch / 2);

        for (int i = 0; i < w; i++)
        {
            for (int j = 0; j < w; j++)
            {
                auto pt = tl_sample + glm::vec2(pitch * i, pitch * j);
                if (membership_check(triangle, pt))
                    samples_inside++;
            }
        }

        return ((float)samples_inside) / spp;
    }

    glm::vec2 pix_to_pt(int x, int y, int w, int h)
    {
        return glm::vec2(-1 + 2 * float(x) / w, -1 + 2 * float(y) / h);
    }

    void Rasterizer::enableDepthTest()
    {
        depth_enabled = true;
        z_buffer = new Uint16[framebuffer->w * framebuffer->h];
    }

    Uint32 vec4_to_color(SDL_PixelFormat *fmt, glm::vec4 color)
    {
        return SDL_MapRGBA(fmt, (Uint8)(color[0] * 255), (Uint8)(color[1] * 255), (Uint8)(color[2] * 255),
                           (Uint8)(color[3] * 255));
    }

    // Clear the framebuffer, setting all pixels to the given color.
    void Rasterizer::clear(glm::vec4 color)
    {
        SDL_FillRect(framebuffer, NULL, vec4_to_color(framebuffer->format, color));
    }

    glm::vec2 buffer_to_vec2(std::vector<float> buf)
    {
        assert(buf.size() >= 2);
        return glm::vec2(buf[0], buf[1]); //, buf[2], buf[3]);
    }
    glm::vec3 buffer_to_vec3(std::vector<float> buf)
    {
        assert(buf.size() >= 3);
        return glm::vec3(buf[0], buf[1], buf[2]); //, buf[2], buf[3]);
    }
    glm::vec4 buffer_to_vec4(std::vector<float> buf)
    {
        assert(buf.size() >= 4);
        return glm::vec4(buf[0], buf[1], buf[2], buf[3]);
    }

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
        // std::cout << depth_enabled << std::endl;
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
        std::cout << vertex_pos[idx.x].x << " " << vertex_pos[idx.x].y << " " << vertex_pos[idx.x].z << " "
                  << vertex_pos[idx.x].w << "\n";
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
                if (membership_check(triangle, pix_tl))
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

                    pixels[(h - j - 1) * w + i] = pix_blend(foreground, background, framebuffer->format);
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
        SDL_BlitScaled(framebuffer, NULL, windowSurface, NULL);
        SDL_UpdateWindowSurface(window);
    }

} // namespace Software
} // namespace COL781
