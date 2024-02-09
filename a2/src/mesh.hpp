#include <vector>
#include "glm/glm.hpp"
#include "viewer.hpp"

/*
 *
 * Define a mesh data structure to store the connectivity and geometry of a 
 * triangle mesh.
 * 1. The connectivity representation should allow efficient access to the 
 *    neighbouring elements of each vertex and of each triangle.
 * 2. Meshes with boundaries should be supported. Even for a boundary vertex, 
 *    it should be straightforward to traverse all its neighbouring triangles.
 * 3. Both positions and normals of all vertices should be stored.
 * 4. You should also implement functionality to send your mesh to the 
 *    rasterization API for rendering, like in Assignment 1.
 * 
 * Further implementation choices beyond this are up to you, e.g. whether to use
 * triangle neighbours (with or without edges) or a half-edge data structure,
 * whether to use indices or pointers, etc.
 */

class Mesh {

    int n_verts, n_edges, n_tris;
    std::vector<int> vert_edges; // [0, n_verts-1] -> [0, n_edges-1]
    std::vector<glm::ivec2> edge_verts; // [0, n_edges-1] -> [0, n_verts-1]
    std::vector<int> edge_tris;  // [0, n_edges-1] -> [0, n_tris-1]

    std::vector<glm::ivec3> tri_edges;  // (tri*3, tri*3 + 1, tri*3 + 2)
    std::vector<glm::ivec3> tri_verts;  // (tri*3, tri*3 + 1, tri*3 + 2)

    // ideally we'd have a VertAttrs struct to store pos and normal, and then 
    // unzip it when passing to the renderer. But since the only attrs we 
    // really care about are position and normal, there's no need to make 
    // such a struct
    std::vector<glm::vec3> vert_pos;     // n_verts
    std::vector<glm::vec3> vert_normal;  // n_verts

    Mesh();
    Mesh(int n_verts, int n_edges, int n_tris);
    void load_objfile(std::string& filename);
    void display_mesh(COL781::Viewer::Viewer& viewer);
};
