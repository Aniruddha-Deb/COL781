#include "viewer.hpp"
#include "mesh.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace V = COL781::Viewer;
using namespace glm;

template <typename T> void print_buffer(std::vector<T>& a, std::string bufname);

template <> void print_buffer(std::vector<int>& a, std::string bufname)
{

    std::cout << bufname << ": " << std::endl;
    for (int i : a)
    {
        std::cout << std::setfill(' ') << std::setw(2) << i << " ";
    }
    std::cout << std::endl;
}

template <> void print_buffer(std::vector<glm::vec3>& a, std::string bufname)
{

    std::cout << bufname << ": " << std::endl;
    for (auto i : a)
    {
        std::cout << std::setfill(' ') << std::setw(2) << "(" << i.x << ", " << i.y << ", " << i.z << ") ";
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
    int m = 2, n = 2;
    std::vector<glm::vec3> vert_pos;
    std::vector<glm::vec3> vert_normals;
    for (int i = 0; i <= n; i++)
    {
        for (int j = 0; j <= m; j++)
        {
            vert_pos.push_back(glm::vec3(-1 + 2 * i / float(n), -1 + 2 * j / float(m), -1));
            vert_normals.push_back(glm::vec3(0, 0, 1));
        }
    }
    std::vector<glm::ivec3> faces;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            faces.push_back(glm::ivec3(i * (m + 1) + j + 1, i * (m + 1) + j, (i + 1) * (m + 1) + j));
            faces.push_back(glm::ivec3(i * (m + 1) + j + 1, (i + 1) * (m + 1) + j, (i + 1) * (m + 1) + j + 1));
        }
    }

    // vert_pos[4].z = 0.0;
    // faces.erase(faces.begin() + 2);
    mesh.set_vert_attribs(vert_pos, vert_normals);

    mesh.set_faces(faces);

    std::cout << "Loaded " << mesh.n_verts << " verts, " << mesh.n_tris << " triangles and " << mesh.n_he
              << " half edges\n";

    print_buffer(mesh.he_next, "he_next");
    print_buffer(mesh.he_pair, "he_pair");
    print_buffer(mesh.he_vert, "he_vert");
    print_buffer(mesh.vert_pos, "vert_pos");
    print_buffer(mesh.vert_normal, "vert_normal");
    print_buffer(mesh.vert_he, "vert_he");

    mesh.recompute_vertex_normals();
    mesh.check_invariants();

    mesh.edge_flip(mesh.vert_he[4]);

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
