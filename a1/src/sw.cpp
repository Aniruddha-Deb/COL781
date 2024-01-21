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
    bool Rasterizer::initialize(const std::string &title, int width, int height, int spp)
    {
        // TODO
        return false;
    }

    // Returns true if the user has requested to quit the program.
    bool Rasterizer::shouldQuit()
    {
        // TODO
        return false;
    }

    /** Shader programs **/

    // Creates a new shader program, i.e. a pair of a vertex shader and a fragment shader.
    ShaderProgram Rasterizer::createShaderProgram(const VertexShader &vs, const FragmentShader &fs)
    {
        // TODO
        ShaderProgram sp;
        return sp;
    }

    // Makes the given shader program active. Future draw calls will use its vertex and fragment shaders.
    void Rasterizer::useShaderProgram(const ShaderProgram &program)
    {
        // TODO
    }

    // Sets the value of a uniform variable.
    // T is only allowed to be float, int, glm::vec2/3/4, glm::mat2/3/4.
    // Use a similar pattern as setVertexAttribs does
    template <typename T> void setUniform(ShaderProgram &program, const std::string &name, T value)
    {
        // TODO
    }

    // Deletes the given shader program.
    void deleteShaderProgram(ShaderProgram &program)
    {
        // TODO
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
        return obj; // itna hi karna hai kya? Shouldn't we return an
                    // object* (this would be copied out?)
    }

    // copied from hw.cpp
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

} // namespace Software
} // namespace COL781
