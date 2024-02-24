#include "viewer.hpp"
#include "mesh.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

namespace V = COL781::Viewer;
using namespace glm;

template<typename T> void print_buffer(std::vector<T>& a, std::string bufname);

template <>
void print_buffer(std::vector<int>& a, std::string bufname) {

    std::cout << bufname << ": " << std::endl;
    for (int i : a) {
        std::cout << std::setfill(' ') << std::setw(2) << i << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char** argv)
{
    V::Viewer v;
    if (!v.initialize("Mesh viewer", 640, 480))
    {
        return EXIT_FAILURE;
    }

    HalfEdgeMesh mesh;
    int m = 8, n = 2; // m=4, n=2 is flat?
    std::vector<glm::vec3> vert_pos;
    std::vector<glm::vec3> vert_normals;
    std::vector<glm::ivec3> faces;

    vert_pos.push_back(glm::vec3(0, 1, 0));
    for (int i = 0; i < n - 1; i++)
    {
        auto phi = M_PI * double(i + 1) / double(n);
        for (int j = 0; j < m; j++)
        {
            auto theta = 2.0 * M_PI * double(j) / double(n);
            auto x = std::sin(phi) * std::cos(theta);
            auto y = std::cos(phi);
            auto z = std::sin(phi) * std::sin(theta);
            vert_pos.push_back(glm::vec3(x, y, z));
        }
    }
    vert_pos.push_back(glm::vec3(0, -1, 0));
    for (int i = 0; i < m; ++i)
    {
        auto i0 = i + 1;
        auto i1 = (i + 1) % m + 1;
        faces.push_back(glm::ivec3(0, i1, i0));
        i0 = i + m * (n - 2) + 1;
        i1 = (i + 1) % m + m * (n - 2) + 1;
        faces.push_back(glm::ivec3((n - 1) * m + 1, i0, i1));
    }
    for (int j = 0; j < n - 2; j++)
    {
        auto j0 = j * m + 1;
        auto j1 = (j + 1) * m + 1;
        for (int i = 0; i < m; i++)
        {
            auto i0 = j0 + i;
            auto i1 = j0 + (i + 1) % m;
            auto i2 = j1 + (i + 1) % m;
            auto i3 = j1 + i;
            faces.push_back(glm::ivec3(i0, i3, i2));
            faces.push_back(glm::ivec3(i0, i2, i1));
        }
    }
    vert_normals.resize(vert_pos.size(), glm::vec3(0, 0, 0));

    mesh.set_vert_attribs(vert_pos, vert_normals);

    mesh.set_faces(faces);

    std::cout << "Loaded " << mesh.n_verts << " verts, " << mesh.n_tris << " triangles and " << mesh.n_he
              << "half edges\n";
    
    print_buffer(mesh.he_next, "he_next");
    print_buffer(mesh.he_pair, "he_pair");
    print_buffer(mesh.he_vert, "he_vert");

    // mesh.recompute_vertex_normals();

    /*
    load_object(argv[1], vertices, normals, triangles);
    int n_verts = vertices.size();
    int n_tris = triangles.size();
    */
    v.setVertices(mesh.n_verts, mesh.vert_pos.data());
    v.setNormals(mesh.n_verts, mesh.vert_normal.data());
    v.setTriangles(mesh.n_tris, mesh.tri_verts.data());

    v.view();
}
