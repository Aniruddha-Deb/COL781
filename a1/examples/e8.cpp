#include "../src/a1.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
// Interesting scene - Load a .obj and render it with lighting for now.

namespace R = COL781::Software;
// namespace R = COL781::Hardware;

using namespace std::chrono;
using namespace glm;

// https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_reflection_model
glm::vec4 blinn_phong_sw_vs(const R::Uniforms &uniforms, const R::Attribs &in, R::Attribs &out)
{

    vec4 inputPosition = in.get<vec4>(0);
    vec4 inputNormal = in.get<vec4>(1);
    // std::cout << inputPosition.x << " " << inputPosition.y << " " << inputPosition.z << std::endl;

    mat4 projection = uniforms.get<mat4>("projection");
    mat4 modelview = uniforms.get<mat4>("modelview");
    mat4 normalMat = uniforms.get<mat4>("normalMat");

    vec4 position = projection * modelview * inputPosition;
    vec4 vertPos4 = modelview * inputPosition;
    vec3 vertPos = vec3(vertPos4) / vertPos4.w;
    vec3 normalInterp = normalize(vec3(normalMat * inputNormal));
    // std::cout << vertPos.x << " " << vertPos.y << " " << vertPos.z << std::endl;

    out.set<vec4>(0, vec4(vertPos, 0));
    out.set<vec4>(1, inputNormal); // this is an approximation we don't need because
                                   // we're doing phong shading and not gouraud
                                   // can normalize the normals just like any other

    return position;
}

glm::vec4 blinn_phong_sw_fs(const R::Uniforms &uniforms, const R::Attribs &in)
{

    const vec3 lightPos = uniforms.get<vec3>("lightPos");         // vec3(1.0, 1.0, 1.0);
    const vec3 lightColor = uniforms.get<vec3>("lightColor");     // vec3(1.0, 1.0, 1.0);
    const float lightPower = uniforms.get<float>("lightPower");   // 40.0;
    const vec3 ambientColor = uniforms.get<vec3>("ambientColor"); // vec3(0.1, 0.0, 0.0);
    const vec3 diffuseColor = uniforms.get<vec3>("diffuseColor"); // vec3(0.5, 0.0, 0.0);
    const vec3 specColor = uniforms.get<vec3>("specColor");       // vec3(1.0, 1.0, 1.0);
    const float shininess = uniforms.get<float>("shininess");     // 16.0;
    const float screenGamma = uniforms.get<float>("screenGamma"); // 2.2;

    vec3 vertPos = vec3(in.get<vec4>(0));
    vec3 normal = vec3(in.get<vec4>(1));

    // std::cout << vertPos.x << "," << vertPos.y << "," << vertPos.z << std::endl;

    vec3 lightDir = lightPos - vertPos;
    float distance = length(lightDir);
    distance = distance * distance;
    lightDir = normalize(lightDir);

    float lambertian = fmax(dot(lightDir, normal), 0.0);
    float specular = 0.0;

    if (lambertian > 0.0)
    {

        vec3 viewDir = normalize(-vertPos);

        // this is blinn phong
        vec3 halfDir = normalize(lightDir + viewDir);
        float specAngle = fmax(dot(halfDir, normal), 0.0);
        specular = pow(specAngle, shininess);

        // this is phong (for comparison)
        /*
        if (mode == 2) {
          vec3 reflectDir = reflect(-lightDir, normal);
          specAngle = fmax(dot(reflectDir, viewDir), 0.0);
          // note that the exponent is different here
          specular = pow(specAngle, shininess/4.0);
        }
        */
    }
    vec3 colorLinear = ambientColor + diffuseColor * lambertian * lightColor * lightPower / distance +
                       specColor * specular * lightColor * lightPower / distance;
    // apply gamma correction (assume ambientColor, diffuseColor and specColor
    // have been linearized, i.e. have no gamma correction in them)
    vec3 colorGammaCorrected = pow(colorLinear, vec3(1.0 / screenGamma));
    colorGammaCorrected.x = fmin(1, fmax(0, colorGammaCorrected.x));
    colorGammaCorrected.y = fmin(1, fmax(0, colorGammaCorrected.y));
    colorGammaCorrected.z = fmin(1, fmax(0, colorGammaCorrected.z));
    // use the gamma corrected color in the fragment
    return vec4(colorGammaCorrected, 1.0);
}

bool load_object(std::string filename, std::vector<vec4> &verts, std::vector<vec4> &normals, std::vector<ivec3> &tris)
{

    std::ifstream objFile(filename);
    if (!objFile.is_open())
    {
        std::cerr << "Unable to open file: " << filename << '\n';
        return false;
    }

    std::string line;
    while (getline(objFile, line))
    {
        if (line.size() == 0)
            continue;
        std::istringstream iss(line);
        std::string token;
        iss >> token;
        if (token == "#")
            continue;
        if (token == "v")
        {
            vec3 v;
            iss >> v.x >> v.y >> v.z;
            verts.push_back(vec4(v, 1.0f));
        }
        else if (token == "vn")
        {
            vec3 n;
            iss >> n.x >> n.y >> n.z;
            normals.push_back(vec4(n, 0.0f));
        }
        else if (token == "f")
        {
            // vertex_index/texture_index/normal_index. Parse.
            ivec3 t;
            ivec3 d;
            std::string e1, e2, e3;
            iss >> e1 >> e2 >> e3;
            sscanf(e1.data(), "%d//%d", &t[0], &d[0]);
            sscanf(e2.data(), "%d//%d", &t[1], &d[1]);
            sscanf(e3.data(), "%d//%d", &t[2], &d[2]);
            t -= 1;
            tris.push_back(t);
        }
    }

    objFile.close();
    return true;
}

glm::vec3 to_vec3(glm::vec4 &v)
{
    return glm::vec3(v[0], v[1], v[2]);
}

int main(int argc, char **argv)
{
    R::Rasterizer r;
    int width = 1280, height = 800;
    if (!r.initialize("Example 5", width, height))
        return EXIT_FAILURE;

    R::ShaderProgram program = r.createShaderProgram(blinn_phong_sw_vs, blinn_phong_sw_fs);

    r.setUniform(program, "lightPos", vec3(1.0, 1.0, 1.0));
    r.setUniform(program, "lightColor", vec3(1.0, 1.0, 1.0));
    r.setUniform(program, "lightPower", 40.0f);
    r.setUniform(program, "ambientColor", vec3(0.3, 0.0, 0.0));
    r.setUniform(program, "diffuseColor", vec3(0.72, 0.1, 0.1));
    r.setUniform(program, "specColor", vec3(0.3, 0.3, 0.3));
    r.setUniform(program, "shininess", 3.3f);
    r.setUniform(program, "screenGamma", 2.2f);

    // load vertices and triangles from an object file
    R::Object shape = r.createObject();
    std::vector<vec4> verts;
    std::vector<vec4> normals;
    std::vector<ivec3> tris;
    if (!load_object(argv[1], verts, normals, tris))
    {
        std::cout << "Could not load object!" << std::endl;
        return EXIT_SUCCESS;
    }
    std::cout << "Loaded object from file" << std::endl;

    std::cout << normals.size() << std::endl;
    std::cout << verts.size() << std::endl;
    std::cout << tris.size() << std::endl;

    r.setVertexAttribs<vec4>(shape, 0, verts.size(), verts.data());
    r.setVertexAttribs<vec4>(shape, 1, normals.size(), normals.data());
    r.setTriangleIndices(shape, tris.size(), tris.data());
    r.enableDepthTest();
    std::cout << "Loaded into buffers" << std::endl;

    // The transformation matrix.
    mat4 model = translate(mat4(1.f), vec3(0.f, 0.f, 0.f));
    mat4 view = translate(mat4(1.0f), vec3(0.f, 0.f, -10.0f));
    mat4 projection = perspective(radians(60.0f), (float)width / (float)height, 0.5f, 100.0f);

    r.clear(vec4(0.1, 0.1, 0.1, 1.0));
    r.useShaderProgram(program);
    float speed = 10.0f; // degrees per second
    float n_frames = 0;
    float max_duration_us = 2e6;
    auto tic = high_resolution_clock::now();
    while (!r.shouldQuit())
    {
        r.clear(vec4(0.1, 0.1, 0.1, 1.0));
        view = rotate(view, radians(30.0f), vec3(1.0f, 0.0f, 0.0f));
        mat4 modelview = view * model;
        mat4 normalMat = transpose(inverse(modelview));

        r.setUniform(program, "projection", projection);
        r.setUniform(program, "modelview", modelview);
        r.setUniform(program, "normalMat", normalMat);

        r.drawObject(shape);
        r.show();

        n_frames += 1;

        auto toc = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(toc - tic).count();
        if (duration > max_duration_us)
        {
            std::cout << "fps: " << 1e6 * n_frames / max_duration_us << std::endl;
            tic = high_resolution_clock::now();
            // r.deleteShaderProgram(program);
            n_frames = 0;
            // return EXIT_SUCCESS;
        }
    }
    r.deleteShaderProgram(program);
    return EXIT_SUCCESS;
}
