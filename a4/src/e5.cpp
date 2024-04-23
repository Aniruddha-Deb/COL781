#include "camera.hpp"
#include "cloth.hpp"
#include "sphere.hpp"
#include "plane.hpp"
#include <iostream>

using namespace COL781;
namespace GL = COL781::OpenGL;
using namespace glm;

GL::Rasterizer r;
GL::ShaderProgram program;

GL::Object object1;
GL::Object object2;
GL::Object object3;
GL::AttribBuf vertexBuf1, normalBuf1;
GL::AttribBuf vertexBuf2, normalBuf2;
GL::AttribBuf vertexBuf3, normalBuf3;

CameraControl camCtl;

int main()
{
    int width = 640, height = 480;
    if (!r.initialize("Animation", width, height))
    {
        return EXIT_FAILURE;
    }
    camCtl.initialize(width, height);
    camCtl.camera.setCameraView(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -0.5f, -1.5f), vec3(0.0, 1.0, 0.0));
    program = r.createShaderProgram(r.vsBlinnPhong(), r.fsBlinnPhong());

    // initializeScene();

    object1 = r.createObject();
    object2 = r.createObject();
    object3 = r.createObject();
    float k_struct = 0.5f;
    float k_shear = 0.1f;
    float k_bend = 0.02f;
    float mass = 1e-3;
    float damping_factor = 0.05;
    Cloth cloth(glm::vec3(-0.5f, 3.0f, -2.5f), glm::vec3(0.5f, 3.0f, -2.5f), glm::vec3(0.5f, -0.0f, -2.0f),
                glm::vec3(-0.5f, -0.0f, -2.0f), 20, 20, k_struct, k_shear, k_bend, damping_factor, mass,
                SDL_GetTicks64() * 1e-3);
    // for (int i = 0; i < cloth.res_w; i++)
    // {
    //     cloth.fix_vertex(0, i);
    // }
    vertexBuf1 = r.createVertexAttribs(object1, 0, cloth.vert_pos.size(), cloth.vert_pos.data());
    normalBuf1 = r.createVertexAttribs(object1, 1, cloth.vert_normals.size(), cloth.vert_normals.data());
    r.createTriangleIndices(object1, cloth.faces.size(), cloth.faces.data());

    Sphere sphere(glm::vec3(0.0, -0.9f, -4.0f), 0.3f, glm::vec3(0.0f, 0.0f, 0.2f), glm::vec3(-2.0f, -2.0f, 8.0f), 0.0f,
                  5.0f, SDL_GetTicks64() * 1e-3);
    vertexBuf2 = r.createVertexAttribs(object2, 0, sphere.vert_pos.size(), sphere.vert_pos.data());
    normalBuf2 = r.createVertexAttribs(object2, 1, sphere.vert_normals.size(), sphere.vert_normals.data());
    r.createTriangleIndices(object2, sphere.faces.size(), sphere.faces.data());

    Plane plane(glm::vec3(0.0f, -1.5f, -4.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.5f, 0.4f, SDL_GetTicks64() * 1e-3);

    vertexBuf3 = r.createVertexAttribs(object3, 0, plane.vert_pos.size(), plane.vert_pos.data());
    normalBuf3 = r.createVertexAttribs(object3, 1, plane.vert_normals.size(), plane.vert_normals.data());
    r.createTriangleIndices(object3, plane.faces.size(), plane.faces.data());

    // cloth.spheres.push_back(sphere);
    cloth.planes.push_back(plane);
    int interpolate = 15;
    while (!r.shouldQuit())
    {
        float t = SDL_GetTicks64() * 1e-3;
        float prev = cloth.time;

        for (int i = 1; i <= interpolate; i++)
        {
            sphere.update(prev + i * ((t - prev) / interpolate));
            cloth.update(prev + i * ((t - prev) / interpolate));
        }
        r.updateVertexAttribs(vertexBuf1, cloth.vert_pos.size(), cloth.vert_pos.data());
        r.updateVertexAttribs(normalBuf1, cloth.vert_normals.size(), cloth.vert_normals.data());
        r.updateVertexAttribs(vertexBuf2, sphere.vert_pos.size(), sphere.vert_pos.data());
        r.updateVertexAttribs(normalBuf2, sphere.vert_normals.size(), sphere.vert_normals.data());

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
        r.drawObject(object1);
        r.drawObject(object2);
        r.drawObject(object3);

        r.setupWireFrame();
        glm::vec3 black(0.0f, 0.0f, 0.0f);
        r.setUniform(program, "ambientColor", black);
        r.setUniform(program, "diffuseColor", black);
        r.setUniform(program, "specularColor", black);
        r.setUniform(program, "phongExponent", 0.f);
        r.drawObject(object1);
        r.drawObject(object2);
        r.drawObject(object3);

        r.show();
    }
}
