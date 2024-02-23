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

    Mesh m;
    std::vector<glm::vec3> vert_pos = {glm::vec3(-1, 1, 0), glm::vec3(1, 1, 0), glm::vec3(1, -1, 0),
                                       glm::vec3(-1, -1, 0)};
    std::vector<glm::vec3> vert_normals = {glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1),
                                           glm::vec3(0, 0, 1)};
    m.set_vert_attribs(vert_pos, vert_normals);
    std::vector<glm::ivec3> faces = {glm::ivec3(0, 3, 2), glm::ivec3(0, 2, 1)};
    m.set_faces(faces);

    std::cout << "Loaded " << m.n_verts << " verts, " << m.n_tris << " triangles and " << m.n_edges << " edges\n";

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
