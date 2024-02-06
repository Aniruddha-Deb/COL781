#include "viewer.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

namespace V = COL781::Viewer;
using namespace glm;

bool load_object(std::string filename, std::vector<vec3> &verts, std::vector<vec3> &normals, std::vector<ivec3> &tris)
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
            verts.push_back(v);
        }
        else if (token == "vn")
        {
            vec3 n;
            iss >> n.x >> n.y >> n.z;
            normals.push_back(n);
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

int main(int argc, char** argv) {
    std::vector<vec3> vertices;
    std::vector<vec3> normals;
    std::vector<ivec3> triangles;
    /*
    vec3 vertices[] = {
        vec3(-0.5, -0.5, 0.0),
        vec3( 0.5, -0.5, 0.0),
        vec3(-0.5,  0.5, 0.0),
        vec3( 0.5,  0.5, 0.0)
    };
    vec3 normals[] = {
        vec3(0.0, 0.0, 1.0),
        vec3(0.0, 0.0, 1.0),
        vec3(0.0, 0.0, 1.0),
        vec3(0.0, 0.0, 1.0)
    };
    ivec3 triangles[] = {
        ivec3(0, 1, 2),
        ivec3(1, 2, 3)
    };
    */

    V::Viewer v;
    if (!v.initialize("Mesh viewer", 640, 480)) {
        return EXIT_FAILURE;
    }

    load_object(argv[1], vertices, normals, triangles);
    int n_verts = vertices.size();
    int n_tris = triangles.size();
    v.setVertices(n_verts, vertices.data());
    v.setNormals(n_verts, normals.data());
    v.setTriangles(n_tris, triangles.data());

    v.view();
}
