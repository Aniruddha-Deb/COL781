#include "viewer.hpp"
#include "mesh.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

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

    std::string path(argv[1]);

    HalfEdgeMesh mesh;
    mesh.load_objfile(path);

    std::cout << "Loaded " << mesh.n_verts << " verts, " << mesh.n_tris << " triangles and " << mesh.n_he
              << " halfedges\n";

    mesh.taubin_smoothing(0.33, -0.34, 10);
    // for (int i = 0; i < 10; i++)
    // {
    //     mesh.gaussian_smoothing(0.33);
    // }
    mesh.recompute_vertex_normals();

    /*
    load_object(argv[1], vertices, normals, triangles);
    int n_verts = vertices.size();
    int n_tris = triangles.size();
    */
    // print_buffer(mesh.he_next, "he_next");
    // print_buffer(mesh.he_pair, "he_pair");
    // print_buffer(mesh.he_vert, "he_vert");
    // print_buffer(mesh.vert_pos, "vert_pos");
    // print_buffer(mesh.vert_normal, "vert_normal");

    v.setVertices(mesh.n_verts, mesh.vert_pos.data());
    v.setNormals(mesh.n_verts, mesh.vert_normal.data());
    v.setTriangles(mesh.n_tris, mesh.tri_verts.data());

    v.view();
}
