#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include "glm/glm.hpp"

#include "mesh.hpp"

void HalfEdgeMesh::load_objfile(std::string &filename)
{

    std::ifstream objFile(filename);
    if (!objFile.is_open())
    {
        std::cerr << "Unable to open file: " << filename << '\n';
    }

    std::string line;
    while (getline(objFile, line))
    {
        if (line.size() == 0)
            continue;
        std::istringstream iss(line);
        std::string token;
        iss >> token;
        if (token == "#")
            continue;
        if (token == "v")
        {
            glm::vec3 v;
            iss >> v.x >> v.y >> v.z;
            vert_pos.push_back(v);
            vert_he.push_back(-1);
        }
        else if (token == "vn")
        {
            glm::vec3 n;
            iss >> n.x >> n.y >> n.z;
            vert_normal.push_back(n);
        }
        else if (token == "f")
        {
            // vertex_index/texture_index/normal_index. Parse.
            glm::ivec3 v, n;
            std::string e1, e2, e3;
            iss >> e1 >> e2 >> e3;
            sscanf(e1.data(), "%d//%d", &v[0], &n[0]);
            sscanf(e2.data(), "%d//%d", &v[1], &n[1]);
            sscanf(e3.data(), "%d//%d", &v[2], &n[2]);
            v -= 1; // obj files are 1-indexed
            n -= 1; // TODO what if normal verts are not the same.
                    // Have a map for this as well
            // assuming all the triangles are counter-clockwise

            // add half-edges
            int tri_idx = tri_he.size();
            int he_start_idx = he_next.size();
            tri_he.push_back(he_start_idx);
            tri_verts.push_back(v);
            he_vert.resize(he_vert.size() + 3);
            he_next.resize(he_next.size() + 3);
            he_pair.resize(he_pair.size() + 3);
            he_tri.resize(he_tri.size() + 3, tri_idx);

            // connect half-edges
            for (int i = 0, j = 1; i < 3; i++, j = (j + 1) % 3)
            {
                int v1 = v[i], v2 = v[j];
                int he_idx = he_start_idx + i;
                he_next[he_idx] = he_start_idx + j;
                he_vert[he_idx] = v2;
                vert_he[v2] = he_idx; // this ensures that vertices always point to
                                      // the most recent halfedge that they're the
                                      // head of. This allows us to traverse in a
                                      // counterclockwise manner if the vertex is
                                      // on the exterior of the mesh rather than
                                      // the interior
                uint64_t key = (uint64_t(v2) << 32) | v1;
                if (he_map.find(key) != he_map.end())
                {
                    int hep = he_map[key];
                    he_pair[he_idx] = hep;
                    he_pair[hep] = he_idx;
                }
                he_map[(uint64_t(v1) << 32) | v2] = he_idx;
            }
        }
    }

    n_verts = vert_pos.size();
    n_tris = tri_he.size();
    n_he = he_vert.size();

    objFile.close();
}

std::vector<int> HalfEdgeMesh::get_adjacent_vertices(int vertex)
{
    // get all adjacent vertices
    // This repeats the first vertex in case the vertex is an interior vertex!
    // see note in mesh loading above: maintaining that invariant ensures
    // we only need to circulate in one direction
    std::vector<int> vertices;
    int he = vert_he[vertex];
    int he_start = he;
    vertices.push_back(he_vert[he_next[he_next[he]]]); // not he_vert[he_pair[he]]
                                                       // as this edge might
                                                       // not have a pair!
    he = he_next[he];
    while (he_pair[he] != -1 && he_pair[he] != he_start)
    {
        vertices.push_back(he_vert[he]);
        he = he_next[he_pair[he]];
    }
    vertices.push_back(he_vert[he]);
    return vertices;
}

void HalfEdgeMesh::gaussian_smoothing(float lambda) {

    std::vector<glm::vec3> vert_pos_new(vert_pos.size());
    for (int v=0; v<n_verts; v++) {
        std::vector<int> nbd = get_adjacent_vertices(v);
        glm::vec3 delta(.0f, .0f, .0f);
        int n_lim = (nbd.back() == nbd.front()) ? nbd.size()-1 : nbd.size();
        for (int i=0; i<n_lim; i++) {
            delta += (vert_pos[nbd[i]] - vert_pos[v]);
        }
        delta /= n_lim;
        vert_pos_new[v] = vert_pos[v] + lambda * delta;
    }
    vert_pos = vert_pos_new;
}

void HalfEdgeMesh::taubin_smoothing(float lambda, float mu, int n_iter) {

    while (n_iter--) {
        gaussian_smoothing(lambda);
        gaussian_smoothing(mu);
    }

}

void HalfEdgeMesh::recompute_vertex_normals() {
    
    // Liu (1999)
    // https://escholarship.org/content/qt7657d8h3/qt7657d8h3.pdf?t=ptt283
    vert_normal.resize(vert_pos.size());
    for (int q = 0; q < n_verts; q++)
    {
        std::vector<int> vertices = get_adjacent_vertices(q);
        glm::vec3 normal(.0f, .0f, .0f);
        for (int i = 0; i < vertices.size() - 1; i++)
        {
            auto v1 = vert_pos[vertices[i]] - vert_pos[q];
            float mv1 = length(v1);
            auto v2 = vert_pos[vertices[i + 1]] - vert_pos[q];
            float mv2 = length(v2);
            normal += glm::cross(v2, v1) / (mv1 * mv1 * mv2 * mv2);
        }
        vert_normal[q] = glm::normalize(normal);
    }
}

void HalfEdgeMesh::set_vert_attribs(std::vector<glm::vec3> &vert_pos, std::vector<glm::vec3> &vert_normal)
{
    assert(vert_pos.size() >= vert_normal.size());
    this->vert_pos = vert_pos;
    this->vert_normal = vert_normal;
    this->n_verts = vert_pos.size();
    this->vert_he.resize(n_verts, -1);
    this->vert_normal.resize(this->n_verts, glm::vec3(0, 0, 0));
}

void HalfEdgeMesh::set_faces(std::vector<glm::ivec3> &faces)
{
    for (auto &face : faces)
    {
        int tri_idx = tri_he.size();
        int he_start_idx = he_next.size();
        tri_he.push_back(he_start_idx);
        tri_verts.push_back(face);
        he_vert.resize(he_vert.size() + 3);
        he_next.resize(he_next.size() + 3);
        he_pair.resize(he_pair.size() + 3);
        he_tri.resize(he_tri.size() + 3, tri_idx);

        // connect half-edges
        for (int i = 0, j = 1; i < 3; i++, j = (j + 1) % 3)
        {
            int v1 = face[i], v2 = face[j];
            assert(v1 <= vert_he.size() && v2 <= vert_he.size());
            int he_idx = he_start_idx + i;
            he_next[he_idx] = he_start_idx + j;
            he_vert[he_idx] = v2;
            vert_he[v2] = he_idx;
            uint64_t key = (uint64_t(v2) << 32) | v1;
            if (he_map.find(key) != he_map.end())
            {
                int hep = he_map[key];
                he_pair[he_idx] = hep;
                he_pair[hep] = he_idx;
            }
            he_map[(uint64_t(v1) << 32) | v2] = he_idx;
        }
    }
    n_tris = tri_he.size();
    n_he = he_vert.size();
}

void Mesh::load_objfile(std::string &filename)
{

    std::ifstream objFile(filename);
    if (!objFile.is_open())
    {
        std::cerr << "Unable to open file: " << filename << '\n';
    }

    std::string line;
    while (getline(objFile, line))
    {
        if (line.size() == 0)
            continue;
        std::istringstream iss(line);
        std::string token;
        iss >> token;
        if (token == "#")
            continue;
        if (token == "v")
        {
            glm::vec3 v;
            iss >> v.x >> v.y >> v.z;
            vert_pos.push_back(v);
            vert_edges.push_back(-1);
        }
        else if (token == "vn")
        {
            glm::vec3 n;
            iss >> n.x >> n.y >> n.z;
            vert_normal.push_back(n);
        }
        else if (token == "f")
        {
            // vertex_index/texture_index/normal_index. Parse.
            int curr_tri_idx = edge_tris.size();
            glm::ivec3 v, n;
            std::string e1, e2, e3;
            iss >> e1 >> e2 >> e3;
            sscanf(e1.data(), "%d//%d", &v[0], &n[0]);
            sscanf(e2.data(), "%d//%d", &v[1], &n[1]);
            sscanf(e3.data(), "%d//%d", &v[2], &n[2]);

            // TODO what do we do if all the triangles are not counter clockwise?
            // The edge storage will get a bit messy here while loading.

            // e1->e2, e2->e3, e3->e1

            v -= 1;
            n -= 1;
            vert_edges[v[0]] = edge_tris.size();
            edge_tris.push_back(tri_verts.size());
            edge_verts.push_back({v[0], v[1]});

            vert_edges[v[1]] = edge_tris.size();
            edge_tris.push_back(tri_verts.size());
            edge_verts.push_back({v[1], v[2]});

            vert_edges[v[2]] = edge_tris.size();
            edge_tris.push_back(tri_verts.size());
            edge_verts.push_back({v[2], v[0]});

            tri_verts.push_back(v);
        }
    }

    n_verts = vert_pos.size();
    n_tris = tri_verts.size();
    n_edges = edge_tris.size();

    objFile.close();
}

void Mesh::set_vert_attribs(std::vector<glm::vec3> &vert_pos, std::vector<glm::vec3> &vert_normal)
{
    assert(vert_pos.size() == vert_normal.size());
    this->vert_pos = vert_pos;
    this->vert_normal = vert_normal;
    this->n_verts = vert_pos.size();
    this->vert_edges.resize(this->n_verts, -1);
}

void Mesh::set_faces(std::vector<glm::ivec3> &faces)
{
    for (auto &face : faces)
    {
        vert_edges[face[0]] = edge_tris.size();
        edge_tris.push_back(tri_verts.size());
        edge_verts.push_back({face[0], face[1]});

        vert_edges[face[1]] = edge_tris.size();
        edge_tris.push_back(tri_verts.size());
        edge_verts.push_back({face[1], face[2]});

        vert_edges[face[2]] = edge_tris.size();
        edge_tris.push_back(tri_verts.size());
        edge_verts.push_back({face[2], face[0]});

        tri_verts.push_back(face);
    }
    n_tris = tri_verts.size();
    n_edges = edge_tris.size();
}
