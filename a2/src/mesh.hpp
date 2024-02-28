#include <vector>
#include <unordered_map>
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

class HalfEdgeMesh
{

  public:
    int n_verts, n_he, n_tris;

    std::vector<int> vert_he;
    std::vector<int> tri_he;
    std::vector<glm::ivec3> tri_verts; // fast rendering
    std::vector<int> he_vert;
    std::vector<int> he_next;
    std::vector<int> he_pair;
    std::vector<int> he_tri;
    std::unordered_map<uint64_t, int> he_map;

    std::vector<glm::vec3> vert_pos;
    std::vector<glm::vec3> vert_normal;

    void load_objfile(std::string &filename);
    void recompute_vertex_normals();
    std::vector<int> get_adjacent_vertices(int vertex);
    void gaussian_smoothing(float lambda);
    void taubin_smoothing(float lambda, float mu, int n_iter);
    void set_vert_attribs(std::vector<glm::vec3> &vert_pos, std::vector<glm::vec3> &vert_normal);
    void set_faces(std::vector<glm::ivec3> &faces);
    void add_face(glm::ivec3 &face);
    void set_boundary();
    bool v_in_tri(int tri, int vertex);
    void check_invariants();
    void edge_flip(int he);
    void edge_split(int he);
    void edge_collapse(int he);
};
