#include "camera.hpp"
#include "cloth.hpp"
#include "timeline.hpp"
#include <iostream>

using namespace COL781;
namespace GL = COL781::OpenGL;
using namespace glm;

GL::Rasterizer r;
GL::ShaderProgram program;

GL::Object object;
GL::AttribBuf vertexBuf, normalBuf;

CameraControl camCtl;
std::vector<vec3> verts;
std::vector<ivec3> tris;

Timeline timeline;

void create_body() {

    const float EPS = 1.f/2;

    std::shared_ptr<Mesh> torso_mesh = std::make_shared<BoxMesh>(glm::vec3(-1.f, 0.f, -.5f)*EPS, glm::vec3(1.f, 3.f, .5f)*EPS);
    std::shared_ptr<Mesh> head_mesh = std::make_shared<BoxMesh>(glm::vec3(-1.f, 0.f, -1.f)*EPS, glm::vec3(1.f, 2.f, 1.f)*EPS);
    std::shared_ptr<Mesh> limb_mesh = std::make_shared<BoxMesh>(glm::vec3(-.5f, -1.5f, -.5f)*EPS, glm::vec3(.5f, 0.f, .5f)*EPS);

    std::shared_ptr<Bone> torso_bone = std::make_shared<Bone>(nullptr, glm::vec3(0.f, 0.f, 0.f)*EPS, glm::vec3(0.f, 0.f, 0.f));
    std::shared_ptr<Bone> head_bone = std::make_shared<Bone>(torso_bone, glm::vec3(0.f, 3.f, 0.f)*EPS, glm::vec3(1.f, 0.f, 0.f));
    std::shared_ptr<Bone> r_upper_arm_bone = std::make_shared<Bone>(torso_bone, glm::vec3(1.5f, 3.f, 0.f)*EPS, glm::vec3(1.f, 0.f, 0.f));
    std::shared_ptr<Bone> r_lower_arm_bone = std::make_shared<Bone>(r_upper_arm_bone, glm::vec3(0.f, -1.5f, 0.f)*EPS, glm::vec3(1.f, 0.f, 0.f));
    std::shared_ptr<Bone> l_upper_arm_bone = std::make_shared<Bone>(torso_bone, glm::vec3(-1.5f, 3.f, 0.f)*EPS, glm::vec3(1.f, 0.f, 0.f));
    std::shared_ptr<Bone> l_lower_arm_bone = std::make_shared<Bone>(l_upper_arm_bone, glm::vec3(0.f, -1.5f, 0.f)*EPS, glm::vec3(1.f, 0.f, 0.f));
    std::shared_ptr<Bone> r_upper_leg_bone = std::make_shared<Bone>(torso_bone, glm::vec3(.5f, 0.f, 0.f)*EPS, glm::vec3(1.f, 0.f, 0.f));
    std::shared_ptr<Bone> r_lower_leg_bone = std::make_shared<Bone>(r_upper_leg_bone, glm::vec3(0.f, -1.5f, 0.f)*EPS, glm::vec3(1.f, 0.f, 0.f));
    std::shared_ptr<Bone> l_upper_leg_bone = std::make_shared<Bone>(torso_bone, glm::vec3(-.5f, 0.f, 0.f)*EPS, glm::vec3(1.f, 0.f, 0.f));
    std::shared_ptr<Bone> l_lower_leg_bone = std::make_shared<Bone>(l_upper_leg_bone, glm::vec3(0.f, -1.5f, 0.f)*EPS, glm::vec3(1.f, 0.f, 0.f));

    timeline.model.push_back({torso_mesh, torso_bone});
    timeline.model.push_back({head_mesh, head_bone});
    timeline.model.push_back({limb_mesh, r_upper_arm_bone});
    timeline.model.push_back({limb_mesh, r_lower_arm_bone});
    timeline.model.push_back({limb_mesh, l_upper_arm_bone});
    timeline.model.push_back({limb_mesh, l_lower_arm_bone});
    timeline.model.push_back({limb_mesh, r_upper_leg_bone});
    timeline.model.push_back({limb_mesh, r_lower_leg_bone});
    timeline.model.push_back({limb_mesh, l_upper_leg_bone});
    timeline.model.push_back({limb_mesh, l_lower_leg_bone});

    std::vector<float> rot_vec(timeline.model.size(), 0.f);
    rot_vec[2] = glm::radians(45.f);
    rot_vec[3] = glm::radians(-45.f);

    std::vector<float> rot_vec_2(timeline.model.size(), 0.f);
    rot_vec_2[2] = glm::radians(-45.f);
    rot_vec_2[3] = glm::radians(45.f);

    timeline.add_frame({0.f, glm::vec3(0.f, 0.f, 0.f), rot_vec});
    timeline.add_frame({2.f, glm::vec3(0.f, 0.f, 0.f), rot_vec_2});
}

void initializeScene()
{
    // TODO create timeline
    create_body();
    object = r.createObject();
    timeline.request_frame(0, verts, tris);
    vertexBuf = r.createVertexAttribs(object, 0, verts.size(), verts.data());
    r.createTriangleIndices(object, tris.size(), tris.data());
}

void updateScene(float t)
{
    // TODO update timeline
    verts.clear();
    tris.clear();
    timeline.request_frame(t, verts, tris);
    r.updateVertexAttribs(vertexBuf, verts.size(), verts.data());
    //r.updateVertexAttribs(normalBuf, nv, normals);
}

int main()
{
    int width = 640, height = 480;
    if (!r.initialize("Animation", width, height))
    {
        return EXIT_FAILURE;
    }
    camCtl.initialize(width, height);
    camCtl.camera.setCameraView(vec3(3.f, 4.f, 3.f), vec3(0.f, 0.f, 0.f), vec3(0.0, 1.0, 0.0));
    program = r.createShaderProgram(r.vsBlinnPhong(), r.fsBlinnPhong());

    initializeScene();

    while (!r.shouldQuit())
    {
        float t = SDL_GetTicks64() * 1e-3;
        // std::cout << t << std::endl;
        updateScene(t);

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
