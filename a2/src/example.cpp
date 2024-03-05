#include "viewer.hpp"
#include "mesh.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

namespace V = COL781::Viewer;
using namespace glm;

int main(int argc, char** argv)
{
    V::Viewer v;
    if (!v.initialize("Mesh viewer", 1000, 1000))
    {
        return EXIT_FAILURE;
    }

    std::string path(argv[1]);

    HalfEdgeMesh mesh;
    mesh.load_objfile(path);

    std::cout << "Loaded " << mesh.n_verts << " verts, " << mesh.n_tris << " triangles and " << mesh.n_he
              << " halfedges\n";

    // mesh.taubin_smoothing(0.33, -0.34, 10);
    // for (int i = 0; i < 10; i++)
    // {
    //     mesh.gaussian_smoothing(0.33);
    // }
    mesh.recompute_vertex_normals();
    // mesh.edge_collapse(1);

    mesh.check_invariants();
    // float l = 0;
    // for (int i = 0; i < mesh.n_he; i++)
    // {
    //     l += mesh.he_length(i);
    // }
    // l /= mesh.n_he;
    // std::cout << l << "\n";
    // mesh.remeshing(l, 0.03);
    // mesh.remeshing(l, 0.03);
    // mesh.remeshing(l, 0.03);
    // mesh.remeshing(l, 0.03);

    // mesh.remeshing(l, 0.03);

    // mesh.remeshing(l, 0.03);
    // mesh.remeshing(l, 0.03);

    // mesh.remeshing(l - 0.02, 0.003);
    // mesh.remeshing(l - 0.02, 0.003);

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
