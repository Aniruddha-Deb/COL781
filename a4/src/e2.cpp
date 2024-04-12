#include "camera.hpp"
#include "cloth.hpp"
#include <iostream>

using namespace COL781;
namespace GL = COL781::OpenGL;
using namespace glm;

GL::Rasterizer r;
GL::ShaderProgram program;

const int nv = 4;
const int nt = 2;
vec3 vertices[nv];
vec3 normals[nv];
ivec3 triangles[nt];

GL::Object object;
GL::AttribBuf vertexBuf, normalBuf;

CameraControl camCtl;

int main()
{
    int width = 640, height = 480;
    if (!r.initialize("Animation", width, height))
    {
        return EXIT_FAILURE;
    }
    camCtl.initialize(width, height);
    camCtl.camera.setCameraView(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -0.5f, -1.0f), vec3(0.0, 1.0, 0.0));
    program = r.createShaderProgram(r.vsBlinnPhong(), r.fsBlinnPhong());

    // initializeScene();

    object = r.createObject();
    // Cloth cloth(glm::vec3(-0.5f, 0.0f, -2.0f), glm::vec3(0.5f, 0.0f, -2.0f), glm::vec3(0.5f, 0.0f, -1.0f),
    //             glm::vec3(-0.5f, 0.0f, -1.0f), 20, 20, 1.0f, 0.5f, 0.1f, 1e-3, SDL_GetTicks64() * 1e-3);
    float k_struct = 1.0f;
    float k_shear = 0.5f;
    float k_bend = 0.1f;
    float mass = 1e-3;
    Cloth cloth(glm::vec3(-0.5f, 0.0f, -2.0f), glm::vec3(0.5f, 0.0f, -2.0f), glm::vec3(0.5f, 0.0f, -1.0f),
                glm::vec3(-0.5f, 0.0f, -1.0f), 20, 20, k_struct, k_shear, k_bend, mass, SDL_GetTicks64() * 1e-3);
    for (int i = 0; i < cloth.res_w; i++)
    {
        cloth.fix_vertex(0, i);
    }
    for (int i = 0; i < cloth.res_h; i++)
    {
        cloth.fix_vertex(i, 0);
    }
    vertexBuf = r.createVertexAttribs(object, 0, cloth.vert_pos.size(), cloth.vert_pos.data());
    normalBuf = r.createVertexAttribs(object, 1, cloth.vert_normals.size(), cloth.vert_normals.data());
    r.createTriangleIndices(object, cloth.faces.size(), cloth.faces.data());

    while (!r.shouldQuit())
    {
        float t = SDL_GetTicks64() * 1e-3;
        cloth.update(t);
        r.updateVertexAttribs(vertexBuf, cloth.vert_pos.size(), cloth.vert_pos.data());
        r.updateVertexAttribs(normalBuf, cloth.vert_normals.size(), cloth.vert_normals.data());

        camCtl.update();
        Camera &camera = camCtl.camera;

        r.clear(vec4(1.0, 1.0, 1.0, 1.0));
        r.enableDepthTest();
        r.useShaderProgram(program);

        r.setUniform(program, "model", glm::mat4(1.0));
        r.setUniform(program, "view", camera.getViewMatrix());
        r.setUniform(program, "projection", camera.getProjectionMatrix());
        r.setUniform(program, "lightPos", camera.position);
        r.setUniform(program, "viewPos", camera.position);
        r.setUniform(program, "lightColor", vec3(1.0f, 1.0f, 1.0f));

        r.setupFilledFaces();
        glm::vec3 orange(1.0f, 0.6f, 0.2f);
        glm::vec3 white(1.0f, 1.0f, 1.0f);
        r.setUniform(program, "ambientColor", 0.4f * orange);
        r.setUniform(program, "diffuseColor", 0.9f * orange);
        r.setUniform(program, "specularColor", 0.8f * white);
        r.setUniform(program, "phongExponent", 100.f);
        r.drawObject(object);

        r.setupWireFrame();
        glm::vec3 black(0.0f, 0.0f, 0.0f);
        r.setUniform(program, "ambientColor", black);
        r.setUniform(program, "diffuseColor", black);
        r.setUniform(program, "specularColor", black);
        r.setUniform(program, "phongExponent", 0.f);
        r.drawObject(object);

        r.show();
    }
}
