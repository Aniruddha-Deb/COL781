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
            he_pair.resize(he_pair.size() + 3, -1);
            he_tri.resize(he_tri.size() + 3, tri_idx);

            // connect half-edges
            for (int i = 0, j = 1; i < 3; i++, j = (j + 1) % 3)
            {
                int v1 = v[i], v2 = v[j];
                int he_idx = he_start_idx + i;
                he_next[he_idx] = he_start_idx + j;
                he_vert[he_idx] = v1;
                vert_he[v1] = he_idx;
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
    objFile.close();
    // giving dummy he pairs to boundary hes
    n_verts = vert_pos.size();
    n_tris = tri_he.size();
    n_he = he_vert.size();
    for (int i = 0; i < n_he; i++)
    {
        if (he_pair[i] == -1)
        {
            int v2 = he_vert[i], v1 = he_vert[he_next[i]];
            he_vert.push_back(v1);
            he_pair.push_back(i);
            he_pair[i] = he_vert.size() - 1;
            he_map[(uint64_t(v1) << 32) | v2] = he_vert.size() - 1;
            he_tri.push_back(-1);
            he_next.push_back(-1);
            vert_he[v1] = he_vert.size() - 1;
        }
    }
    for (int i = n_he; i < he_vert.size(); i++)
    {
        he_next[i] = vert_he[he_vert[he_pair[i]]];
    }
    n_verts = vert_pos.size();
    n_tris = tri_he.size();
    n_he = he_vert.size();
}

std::vector<int> HalfEdgeMesh::get_adjacent_vertices(int vertex)
{
    std::vector<int> vertices;
    int he = vert_he[vertex];
    int he_start = he;
    vertices.push_back(he_vert[he_pair[he]]); // now it has a pair!
    he = he_next[he_pair[he]];
    while (he != he_start)
    {
        vertices.push_back(he_vert[he_pair[he]]);
        he = he_next[he_pair[he]];
    }
    if (he_tri[vert_he[vertex]] != -1)
    {
        vertices.push_back(he_vert[he_pair[he_start]]);
    }
    return vertices;
}

void HalfEdgeMesh::gaussian_smoothing(float lambda)
{

    std::vector<glm::vec3> vert_pos_new(vert_pos.size());
    for (int v = 0; v < n_verts; v++)
    {
        std::vector<int> nbd = get_adjacent_vertices(v);
        glm::vec3 delta(.0f, .0f, .0f);
        int n_lim = (nbd.back() == nbd.front()) ? nbd.size() - 1 : nbd.size();
        for (int i = 0; i < n_lim; i++)
        {
            delta += (vert_pos[nbd[i]] - vert_pos[v]);
        }
        delta /= n_lim;
        vert_pos_new[v] = vert_pos[v] + lambda * delta;
    }
    vert_pos = vert_pos_new;
}

void HalfEdgeMesh::taubin_smoothing(float lambda, float mu, int n_iter)
{

    while (n_iter--)
    {
        gaussian_smoothing(lambda);
        gaussian_smoothing(mu);
    }
}

void HalfEdgeMesh::recompute_vertex_normals()
{

    // Liu (1999)
    // https://escholarship.org/content/qt7657d8h3/qt7657d8h3.pdf?t=ptt283
    vert_normal.resize(vert_pos.size());
    for (int q = 0; q < n_verts; q++)
    {
        std::vector<int> vertices = get_adjacent_vertices(q);
        std::cout << "neighbours of " << q << " ";
        for (int i : vertices)
        {
            std::cout << i << " ";
        }
        std::cout << "\n";
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

void HalfEdgeMesh::add_face(glm::ivec3 &face)
{
    int tri_idx = tri_he.size();
    int he_start_idx = he_next.size();
    tri_he.push_back(he_start_idx);
    tri_verts.push_back(face);
    he_vert.resize(he_vert.size() + 3);
    he_next.resize(he_next.size() + 3);
    he_pair.resize(he_pair.size() + 3, -1);
    he_tri.resize(he_tri.size() + 3, tri_idx);

    // connect half-edges
    for (int i = 0, j = 1; i < 3; i++, j = (j + 1) % 3)
    {
        int v1 = face[i], v2 = face[j];
        int he_idx = he_start_idx + i;
        he_next[he_idx] = he_start_idx + j;
        he_vert[he_idx] = v1;
        vert_he[v1] = he_idx;
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

void HalfEdgeMesh::set_boundary()
{
    // giving dummy he pairs to boundary hes
    n_he = he_vert.size();
    for (int i = 0; i < n_he; i++)
    {
        if (he_pair[i] == -1)
        {
            int v2 = he_vert[i], v1 = he_vert[he_next[i]];
            he_vert.push_back(v1);
            he_pair.push_back(i);
            he_pair[i] = he_vert.size() - 1;
            he_map[(uint64_t(v1) << 32) | v2] = he_vert.size() - 1;
            he_tri.push_back(-1);
            he_next.push_back(-1);
            vert_he[v1] = he_vert.size() - 1;
        }
    }
    for (int i = n_he; i < he_vert.size(); i++)
    {
        he_next[i] = vert_he[he_vert[he_pair[i]]];
    }
    n_verts = vert_pos.size();
    n_tris = tri_he.size();
    n_he = he_vert.size();
}

void HalfEdgeMesh::set_faces(std::vector<glm::ivec3> &faces)
{
    for (auto &face : faces)
    {
        add_face(face);
    }
    set_boundary();
}

// void HalfEdgeMesh::edge_flip(int he)
// {
//     int pair = he_pair[he];
// }
