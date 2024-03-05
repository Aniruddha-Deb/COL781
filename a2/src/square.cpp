#include "viewer.hpp"
#include "mesh.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace V = COL781::Viewer;
using namespace glm;

int main(int argc, char** argv)
{
    V::Viewer v;
    if (!v.initialize("Mesh viewer", 640, 480))
    {
        return EXIT_FAILURE;
    }

    HalfEdgeMesh mesh;
    int m = 10, n = 10;
    std::vector<glm::vec3> vert_pos;
    std::vector<glm::vec3> vert_normals;
    for (int i = 0; i <= n; i++)
    {
        for (int j = 0; j <= m; j++)
        {
            vert_pos.push_back(glm::vec3(-1 + 2 * i / float(n), -1 + 2 * j / float(m), -0.5));
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

    // std::cout << mesh.he_next.size() << "\n";
    // int he = mesh.he_next[21];
    // while (he != 21)
    // {
    //     std::cout << he << "\n";
    //     he = mesh.he_next[he];
    // }
    // mesh.edge_split(mesh.he_map[(uint64_t(1) << 32) | 2]);
    // mesh.edge_collapse(27);
    // mesh.edge_flip(mesh.he_map[(uint64_t(6) << 32) | 7]);

    // l = 0;
    // for (int i = 0; i < mesh.n_he; i++)
    // {
    //     l += mesh.he_length(i);
    // }
    // l /= mesh.n_he;
    // mesh.remeshing(l - 0.02, 0.33);

    // l = 0;
    // for (int i = 0; i < mesh.n_he; i++)
    // {
    //     l += mesh.he_length(i);
    // }
    // l /= mesh.n_he;
    // mesh.remeshing(l - 0.02, 0.33);
    // mesh.edge_flip(27);
    // mesh.edge_collapse(mesh.he_map[(uint64_t(1) << 32) | 2]);
    // std::cout << mesh.flip_check(mesh.he_map[(uint64_t(10) << 32) | 13]) << "\n";

    // float l = 0;
    // for (int i = 0; i < mesh.n_he; i++)
    // {
    //     l += mesh.he_length(i);
    // }
    // l /= mesh.n_he;
    // std::cout << l << "\n";
    // mesh.remeshing(l - 0.2, 0.004);
    // mesh.remeshing(l - 0.2, 0.0);
    // mesh.remeshing(l - 0.2, 0.0);
    // mesh.remeshing(l - 0.22, 0.0);

    // mesh.remeshing(l - 0.02, 0.004);

    // mesh.edge_split(27);
    // mesh.edge_collapse(27);
    mesh.recompute_vertex_normals();
    // mesh.check_invariants();

    // mesh.edge_flip(27);
    // mesh.edge_split(27);

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
