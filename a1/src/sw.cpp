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

    template <typename T> T Uniforms::get(const std::string &name) const
    {
        return *(T *)values.at(name);
    }

    template <typename T> void Uniforms::set(const std::string &name, T value)
    {
        auto it = values.find(name);
        if (it != values.end())
        {
            delete it->second;
        }
        values[name] = (void *)(new T(value));
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
            window = SDL_CreateWindow("COL781", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight,
                                      SDL_WINDOW_SHOWN);
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
                quit = true;
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
        return sp;
    }

    // Makes the given shader program active. Future draw calls will use its vertex and fragment shaders.
    void Rasterizer::useShaderProgram(const ShaderProgram &program)
    {
        shader_program = program;
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
        ShaderProgram sp;
        shader_program = sp;
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
        return obj; 
    }

    void setAttribs(Object &object, int attribIndex, int n, int dim, const float *data)
    {
        if (n >= object.attributeValues.size())
        {
            object.attributeValues.resize(n + 1);
            object.attributeDims.resize(n + 1);
        }
        Object::Buffer b;
        for (int i = 0; i < dim; i++)
        {
            b[i] = data[i];
        }
        object.attributeValues[n] = b;
        object.attributeDims[n] = dim;
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

    // Sets the indices of the triangles.
    void Rasterizer::setTriangleIndices(Object &object, int n, glm::ivec3 *indices)
    {
        if (n >= object.indices.size())
        {
            object.indices.resize(n + 1);
        }
        object.indices[n] = *indices;
    }

    /** Drawing **/

    /*
     * Our old SDL triangle rendering code
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

    // sampling will need to be adaptive this time, based on spp.
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

    void render_triangles(std::vector<glm::vec2> &verts, std::vector<glm::vec3> &idxs)
    {

        render_background();
        for (auto idx : idxs)
        {
            glm::vec2 triangle[3] = {verts[idx.x], verts[idx.y], verts[idx.z]};
            render_naive(triangle);
        }
    }
    */

    // Enable depth testing.
    void Rasterizer::enableDepthTest() {
        // set boolean for this in rasterizer?
    }

    // Clear the framebuffer, setting all pixels to the given color.
    void Rasterizer::clear(glm::vec4 color) {
        SDL_FillRect(framebuffer, NULL, SDL_MapRGBA(framebuffer->format, color[0], color[1], color[2], color[3]));
    }

    // Draws the triangles of the given object.
    void Rasterizer::drawObject(const Object &object) {
        // Implement the rendering pipeline here
        // object --vs--> verts in NDC --rasterize--> fragments --fs--> colors
    }

    // Displays the framebuffer on the screen.
    void Rasterizer::show() {
        auto windowSurface = SDL_GetWindowSurface(window);
        SDL_BlitScaled(framebuffer, NULL, windowSurface, NULL);
        SDL_UpdateWindowSurface(window);
    }

} // namespace Software
} // namespace COL781
