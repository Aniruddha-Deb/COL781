#include "viewer.hpp"
#include "mesh.hpp"
#include <iostream>
#include <iomanip>
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

    std::string path(argv[1]);

    HalfEdgeMesh m;
    m.load_objfile(path);

    std::cout << "Loaded " << m.n_verts << " verts, " << m.n_tris << " triangles and " << m.n_he << " halfedges\n";
    m.recompute_vertex_normals();

    /*
    load_object(argv[1], vertices, normals, triangles);
    int n_verts = vertices.size();
    int n_tris = triangles.size();
    */
    v.setVertices(m.n_verts, m.vert_pos.data());
    v.setNormals(m.n_verts, m.vert_normal.data());
    v.setTriangles(m.n_tris, m.tri_verts.data());

    v.view();
}
