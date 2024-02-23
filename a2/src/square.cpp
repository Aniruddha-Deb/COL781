#include "viewer.hpp"
#include "mesh.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

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

    mesh.set_vert_attribs(vert_pos, vert_normals);

    mesh.set_faces(faces);

    std::cout << "Loaded " << mesh.n_verts << " verts, " << mesh.n_tris << " triangles and " << mesh.n_he
              << " half edges\n";

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
