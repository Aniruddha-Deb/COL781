#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_set>
#include "glm/glm.hpp"

#include "mesh.hpp"

#define edge(from, to) ((uint64_t((from)) << 32) | (to))

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
            n_verts++;
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
            add_face(v);
        }
    }
    objFile.close();
    // giving dummy he pairs to boundary hes
    set_boundary();
}

std::vector<int> HalfEdgeMesh::get_adjacent_vertices(int vertex)
{
    std::vector<int> vertices;
    int he = vert_he[vertex];
    int he_start = he;
    vertices.push_back(he_vert[he_pair[he]]);
    he = he_next[he_pair[he]];
    while (he != he_start)
    {
        vertices.push_back(he_vert[he_pair[he]]);
        he = he_next[he_pair[he]];
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
        for (int i = 0; i < nbd.size(); i++)
        {
            delta += (vert_pos[nbd[i]] - vert_pos[v]);
        }
        delta /= nbd.size();
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
        glm::vec3 normal(.0f, .0f, .0f);
        for (int i = 0; i < vertices.size(); i++)
        {
            if (he_map.find((uint64_t(vertices[(i + 1) % vertices.size()]) << 32) | vertices[i]) != he_map.end())
            {
                int he = he_map[(uint64_t(vertices[(i + 1) % vertices.size()]) << 32) | vertices[i]];
                int tri = he_tri[he];
                if ((tri != -1) && (tri_verts[tri][0] == q || tri_verts[tri][1] == q || tri_verts[tri][2] == q))
                {
                    auto v1 = vert_pos[vertices[i]] - vert_pos[q];
                    float mv1 = length(v1);
                    auto v2 = vert_pos[vertices[(i + 1) % vertices.size()]] - vert_pos[q];
                    float mv2 = length(v2);
                    normal += glm::cross(v2, v1) / (mv1 * mv1 * mv2 * mv2);
                }
            }
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

int HalfEdgeMesh::he_prev(int he)
{
    int prev = he;
    while (he_next[prev] != he)
    {
        // if (he == 36385)
        // {
        //     std::cout << "prev " << prev << "\n";
        // }
        prev = he_next[prev];
    }
    return prev;
}

void HalfEdgeMesh::edge_flip(int he)
{
    // check if edge is not boundary
    assert(he_tri[he] != -1 && he_tri[he_pair[he]] != -1);
    // get pointers
    int next = he_next[he];
    int prev = he_prev(he);
    int pair = he_pair[he];
    int pair_next = he_next[pair];
    int pair_prev = he_prev(pair);
    int origin = he_vert[he];
    int pair_origin = he_vert[pair];
    int new_origin = he_vert[pair_prev];
    int new_pair_origin = he_vert[prev];
    int tri = he_tri[he];
    int pair_tri = he_tri[pair];

    // convert he to new_origin->new_pair_orign
    // convert pair to new_pair_origin->origin
    vert_he[origin] = pair_next;
    vert_he[pair_origin] = next;
    vert_he[new_origin] = he;
    vert_he[new_pair_origin] = pair;
    tri_he[tri] = he;
    tri_he[pair_tri] = pair;
    tri_verts[tri] = glm::ivec3(origin, new_origin, new_pair_origin);
    tri_verts[pair_tri] = glm::ivec3(pair_origin, new_pair_origin, new_origin);
    he_vert[he] = new_origin;
    he_vert[pair] = new_pair_origin;
    he_next[he] = prev;
    he_next[prev] = pair_next;
    he_next[pair_next] = he;
    he_next[pair] = pair_prev;
    he_next[pair_prev] = next;
    he_next[next] = pair;
    he_pair[he] = pair;
    he_pair[pair] = he;
    he_tri[prev] = tri;
    he_tri[pair_next] = tri;
    he_tri[pair_prev] = pair_tri;
    he_tri[next] = pair_tri;
    he_map.erase((uint64_t(origin) << 32) | pair_origin);
    he_map.erase((uint64_t(pair_origin) << 32) | origin);
    he_map[(uint64_t(new_origin) << 32) | new_pair_origin] = he;
    he_map[(uint64_t(new_pair_origin) << 32) | new_origin] = pair;
}

void HalfEdgeMesh::edge_split(int he)
{

    // flip he to get he on the boundary
    if (he_tri[he_pair[he]] == -1)
    {
        he = he_pair[he];
    }

    // get pointers
    int next = he_next[he];
    int prev = he_prev(he);
    int pair = he_pair[he];
    int pair_next = he_next[pair];
    int pair_prev = he_prev(pair);
    int origin = he_vert[he];
    int pair_origin = he_vert[pair];
    int prev_origin = he_vert[prev];
    int pair_prev_origin = he_vert[pair_prev];
    int tri = he_tri[he];
    int pair_tri = he_tri[pair];

    if (tri != -1)
    {
        vert_pos.push_back((vert_pos[origin] + vert_pos[pair_origin]) / 2.0f);
        vert_normal.push_back(glm::vec3(0, 0, 0));
        vert_he.push_back(he);
        vert_he[origin] = n_he;
        vert_he[pair_origin] = pair;
        vert_he[prev_origin] = prev;
        vert_he[pair_prev_origin] = pair_prev;
        tri_he[pair_tri] = pair;
        tri_he[tri] = he;
        tri_he.resize(n_tris + 2);
        tri_he[n_tris] = pair_next;
        tri_he[n_tris + 1] = prev;
        tri_verts.resize(n_tris + 2);
        tri_verts[tri] = glm::ivec3(n_verts, pair_origin, prev_origin);
        tri_verts[pair_tri] = glm::ivec3(n_verts, pair_prev_origin, pair_origin);
        tri_verts[n_tris] = glm::ivec3(n_verts, origin, pair_prev_origin);
        tri_verts[n_tris + 1] = glm::ivec3(n_verts, prev_origin, origin);
        he_vert.resize(n_he + 6);
        he_vert[he] = n_verts;
        he_vert[pair] = pair_origin;
        he_vert[n_he] = origin;
        he_vert[n_he + 1] = n_verts;
        he_vert[n_he + 2] = pair_prev_origin;
        he_vert[n_he + 3] = n_verts;
        he_vert[n_he + 4] = prev_origin;
        he_vert[n_he + 5] = n_verts;
        he_next.resize(n_he + 6);
        he_next[next] = n_he + 4;
        he_next[n_he + 4] = he;
        he_next[pair] = n_he + 3;
        he_next[n_he + 3] = pair_prev;
        he_next[pair_prev] = pair;
        he_next[pair_next] = n_he + 2;
        he_next[n_he + 2] = n_he + 1;
        he_next[n_he + 1] = pair_next;
        he_next[prev] = n_he;
        he_next[n_he] = n_he + 5;
        he_next[n_he + 5] = prev;
        he_pair.resize(n_he + 6);
        he_pair[he] = pair;
        he_pair[pair] = he;
        he_pair[n_he] = n_he + 1;
        he_pair[n_he + 1] = n_he;
        he_pair[n_he + 2] = n_he + 3;
        he_pair[n_he + 3] = n_he + 2;
        he_pair[n_he + 4] = n_he + 5;
        he_pair[n_he + 5] = n_he + 4;
        he_tri.resize(n_he + 6);
        he_tri[n_he + 4] = tri;
        he_tri[n_he + 3] = pair_tri;
        he_tri[prev] = n_tris + 1;
        he_tri[n_he] = n_tris + 1;
        he_tri[n_he + 5] = n_tris + 1;
        he_tri[pair_next] = n_tris;
        he_tri[n_he + 2] = n_tris;
        he_tri[n_he + 1] = n_tris;
        he_map.erase((uint64_t(origin) << 32) | pair_origin);
        he_map.erase((uint64_t(pair_origin) << 32) | origin);
        he_map[(uint64_t(n_verts) << 32) | origin] = n_he + 1;
        he_map[(uint64_t(n_verts) << 32) | prev_origin] = n_he + 5;
        he_map[(uint64_t(n_verts) << 32) | pair_origin] = he;
        he_map[(uint64_t(n_verts) << 32) | pair_prev_origin] = n_he + 3;
        he_map[(uint64_t(origin) << 32) | n_verts] = n_he;
        he_map[(uint64_t(prev_origin) << 32) | n_verts] = n_he + 4;
        he_map[(uint64_t(pair_origin) << 32) | n_verts] = pair;
        he_map[(uint64_t(pair_prev_origin) << 32) | n_verts] = n_he + 2;
        n_verts++;
        n_he += 6;
        n_tris += 2;
    }
    else // boundary case
    {
        vert_pos.push_back((vert_pos[origin] + vert_pos[pair_origin]) / 2.0f);
        vert_normal.push_back(glm::vec3(0, 0, 0));
        vert_he.push_back(he);
        vert_he[origin] = n_he;
        vert_he[pair_origin] = pair;
        vert_he[prev_origin] = prev;
        vert_he[pair_prev_origin] = pair_prev;
        tri_he[pair_tri] = pair;
        tri_he.resize(n_tris + 1);
        tri_he[n_tris] = pair_next;
        tri_verts.resize(n_tris + 1);
        tri_verts[pair_tri] = glm::ivec3(n_verts, pair_prev_origin, pair_origin);
        tri_verts[n_tris] = glm::ivec3(n_verts, origin, pair_prev_origin);
        he_vert.resize(n_he + 4);
        he_vert[he] = n_verts;
        he_vert[pair] = pair_origin;
        he_vert[n_he] = origin;
        he_vert[n_he + 1] = n_verts;
        he_vert[n_he + 2] = pair_prev_origin;
        he_vert[n_he + 3] = n_verts;
        he_next.resize(n_he + 4);
        he_next[pair] = n_he + 3;
        he_next[n_he + 3] = pair_prev;
        he_next[pair_prev] = pair;
        he_next[pair_next] = n_he + 2;
        he_next[n_he + 2] = n_he + 1;
        he_next[n_he + 1] = pair_next;
        he_next[prev] = n_he;
        he_next[n_he] = he;
        he_pair.resize(n_he + 4);
        he_pair[n_he] = n_he + 1;
        he_pair[n_he + 1] = n_he;
        he_pair[n_he + 2] = n_he + 3;
        he_pair[n_he + 3] = n_he + 2;
        he_tri.resize(n_he + 4);
        he_tri[n_he + 3] = pair_tri;
        he_tri[n_he] = -1;
        he_tri[pair_next] = n_tris;
        he_tri[n_he + 2] = n_tris;
        he_tri[n_he + 1] = n_tris;
        he_map.erase((uint64_t(origin) << 32) | pair_origin);
        he_map.erase((uint64_t(pair_origin) << 32) | origin);
        he_map[(uint64_t(n_verts) << 32) | origin] = n_he + 1;
        he_map[(uint64_t(n_verts) << 32) | pair_origin] = he;
        he_map[(uint64_t(n_verts) << 32) | pair_prev_origin] = n_he + 3;
        he_map[(uint64_t(origin) << 32) | n_verts] = n_he;
        he_map[(uint64_t(pair_origin) << 32) | n_verts] = pair;
        he_map[(uint64_t(pair_prev_origin) << 32) | n_verts] = n_he + 2;
        n_verts++;
        n_he += 4;
        n_tris += 1;
    }
}

bool HalfEdgeMesh::v_in_tri(int tri, int v)
{
    return (tri != -1) && (tri_verts[tri][0] == v || tri_verts[tri][1] == v || tri_verts[tri][2] == v);
}

bool HalfEdgeMesh::collapse_check(int he)
{
    // https://www.merlin.uzh.ch/contributionDocument/download/14550
    int pair = he_pair[he];
    int v1 = he_vert[he];
    int v2 = he_vert[pair];
    int tri1 = he_tri[he];
    int tri2 = he_tri[pair];
    std::vector<int> vertices1 = get_adjacent_vertices(v1);
    std::vector<int> vertices2 = get_adjacent_vertices(v2);
    sort(vertices1.begin(), vertices1.end());
    sort(vertices2.begin(), vertices2.end());
    std::vector<int> common;
    std::set_intersection(vertices1.begin(), vertices1.end(), vertices2.begin(), vertices2.end(),
                          std::back_inserter(common));
    bool boundary_1 = false, boundary_2 = false;
    for (int i = 0; i < vertices1.size(); i++)
    {
        int v1_i = he_map[(uint64_t(v1) << 32) | vertices1[i]];
        int v1_i_pair = he_pair[v1_i];
        if (tri_he[v1_i] == -1 || tri_he[v1_i_pair] == -1)
        {
            boundary_1 = true;
            break;
        }
    }
    for (int i = 0; i < vertices2.size(); i++)
    {
        int v2_i = he_map[(uint64_t(v2) << 32) | vertices2[i]];
        int v2_i_pair = he_pair[v2_i];
        if (tri_he[v2_i] == -1 || tri_he[v2_i_pair] == -1)
        {
            boundary_2 = true;
            break;
        }
    }
    bool checks = true;
    checks &= !(boundary_1 || boundary_2);
    checks &= (common.size() == 2);

    for (int k : common)
    {
        bool flag = false;
        if (tri1 != -1)
        {
            flag |= (tri_verts[tri1][0] == k || tri_verts[tri1][1] == k || tri_verts[tri1][2] == k);
        }
        if (tri2 != -1)
        {
            flag |= (tri_verts[tri2][0] == k || tri_verts[tri2][1] == k || tri_verts[tri2][2] == k);
        }
        checks &= flag;
    }
    if (common.size() == 2)
    {
        checks &= (he_map.find((uint64_t(common[0]) << 32) | common[1]) == he_map.end());
    }

    checks &= (n_verts >= 4);

    // geometric checks
    vertices1.erase(std::find(vertices1.begin(), vertices1.end(), v2));
    vertices2.erase(std::find(vertices2.begin(), vertices2.end(), v1));

    for (int i = 0; i < vertices1.size(); i++)
    {
        if (he_map.find((uint64_t(vertices1[(i + 1) % vertices1.size()]) << 32) | vertices1[i]) != he_map.end())
        {
            int i_he = he_map[(uint64_t(vertices1[(i + 1) % vertices1.size()]) << 32) | vertices1[i]];
            int i_tri = he_tri[i_he];
            int i_pair = he_pair[i_he];
            int i_tri_pair = he_tri[i_pair];

            glm::vec3 normal =
                glm::cross(glm::normalize(vert_pos[vertices1[(i + 1) % vertices1.size()]] - vert_pos[vertices1[i]]),
                           glm::cross(glm::normalize(vert_pos[vertices1[i]] - vert_pos[v1]),
                                      glm::normalize(vert_pos[vertices1[(i + 1) % vertices1.size()]] - vert_pos[v1])));
            checks &= (glm::length(normal) > 0.3);
            checks &= (glm::dot(normalize(normal), glm::normalize(vert_pos[vertices1[i]] - vert_pos[v1])) *
                           glm::dot(normalize(normal),
                                    glm::normalize(vert_pos[vertices1[i]] - (vert_pos[v1] + vert_pos[v2]) / 2.0f)) >
                       0.01);
        }
    }

    for (int i = 0; i < vertices2.size(); i++)
    {
        if (he_map.find((uint64_t(vertices2[(i + 1) % vertices2.size()]) << 32) | vertices2[i]) != he_map.end())
        {
            int i_he = he_map[(uint64_t(vertices2[(i + 1) % vertices2.size()]) << 32) | vertices2[i]];
            int i_tri = he_tri[i_he];
            int i_pair = he_pair[i_he];
            int i_tri_pair = he_tri[i_pair];

            glm::vec3 normal =
                glm::cross(glm::normalize(vert_pos[vertices2[(i + 1) % vertices2.size()]] - vert_pos[vertices2[i]]),
                           glm::cross(glm::normalize(vert_pos[vertices2[i]] - vert_pos[v2]),
                                      glm::normalize(vert_pos[vertices2[(i + 1) % vertices2.size()]] - vert_pos[v2])));
            checks &= (glm::length(normal) > 0.3);
            checks &= (glm::dot(normalize(normal), glm::normalize(vert_pos[vertices2[i]] - vert_pos[v2])) *
                           glm::dot(normalize(normal),
                                    glm::normalize(vert_pos[vertices2[i]] - (vert_pos[v2] + vert_pos[v2]) / 2.0f)) >
                       0.01);
        }
    }
    for (int i = 0; i < common.size(); i++)
    {
        vertices1.erase(std::find(vertices1.begin(), vertices1.end(), common[i]));
        vertices2.erase(std::find(vertices2.begin(), vertices2.end(), common[i]));
    }
    checks &= !(vertices1.empty() || vertices2.empty());

    return checks;
}

void HalfEdgeMesh::edge_collapse(int he)
{
    // check if edge can be collapsed
    int pair = he_pair[he];
    int v1 = he_vert[he];
    int v2 = he_vert[pair];
    std::vector<int> vertices1 = get_adjacent_vertices(v1);
    std::vector<int> vertices2 = get_adjacent_vertices(v2);
    sort(vertices1.begin(), vertices1.end());
    sort(vertices2.begin(), vertices2.end());
    std::vector<int> common;
    std::set_intersection(vertices1.begin(), vertices1.end(), vertices2.begin(), vertices2.end(),
                          std::back_inserter(common));
    vertices1.erase(std::find(vertices1.begin(), vertices1.end(), v2));
    vertices2.erase(std::find(vertices2.begin(), vertices2.end(), v1));
    for (int i = 0; i < common.size(); i++)
    {
        vertices1.erase(std::find(vertices1.begin(), vertices1.end(), common[i]));
        vertices2.erase(std::find(vertices2.begin(), vertices2.end(), common[i]));
    }

    for (int i = 0; i < common.size(); i++)
    {
        vert_he[common[i]] = he_map[(uint64_t(common[i]) << 32) | v1];
    }
    // get relevant pointers
    int tri1 = he_tri[he];
    int tri2 = he_tri[pair];
    int next_he = he_next[he];
    int prev_he = he_prev(he);
    int next_pair = he_next[pair];
    int prev_pair = he_prev(pair);

    vert_he[v1] = he_map[(uint64_t(v1) << 32) | common[0]];
    // handle he_next continuity when he is a boundary edge
    if (tri1 == -1)
    {
        he_next[prev_he] = next_he;
    }
    if (tri2 == -1)
    {
        he_next[prev_pair] = next_pair;
    }
    for (int i = 0; i < vertices2.size(); i++)
    {
        int v2_i = he_map[(uint64_t(v2) << 32) | vertices2[i]];
        int v2_i_pair = he_pair[v2_i];
        int tri = he_tri[v2_i];
        int tri_pair = he_tri[v2_i_pair];
        int v2_i_prev = he_prev(v2_i);
        int v2_i_pair_prev = he_prev(v2_i_pair);
        assert(tri == -1 || (tri != tri1 && tri != tri2));
        assert(tri_pair == -1 || (tri_pair != tri1 && tri_pair != tri2));
        if (tri != -1)
        {
            tri_he[tri] = v2_i;
        }
        if (tri_pair != -1)
        {
            tri_he[tri_pair] = v2_i_pair;
        }
        for (int idx = 0; idx < 3; idx++)
        {
            if (tri != -1 && tri_verts[tri][idx] == v2)
            {
                tri_verts[tri][idx] = v1;
            }
            if (tri_pair != -1 && tri_verts[tri_pair][idx] == v2)
            {
                tri_verts[tri_pair][idx] = v1;
            }
        }
        for (int idx = 0; idx < common.size(); idx++)
        {
            if (he_vert[v2_i_prev] == common[idx])
            {
                // std::cout << common[idx] << "detected";
                he_next[he_prev(v2_i_prev)] = he_map[(uint64_t(common[idx]) << 32) | v1];
                he_next[he_map[(uint64_t(common[idx]) << 32) | v1]] = v2_i;
                assert(he_tri[he_map[(uint64_t(common[idx]) << 32) | v1]] == tri1 ||
                       he_tri[he_map[(uint64_t(common[idx]) << 32) | v1]] == tri2);
                he_tri[he_map[(uint64_t(common[idx]) << 32) | v1]] = tri;
            }
            if (he_vert[he_next[he_next[v2_i_pair]]] == common[idx])
            {
                he_next[he_map[(uint64_t(v1) << 32) | common[idx]]] = he_next[he_next[v2_i_pair]];
                he_next[v2_i_pair] = he_map[(uint64_t(v1) << 32) | common[idx]];
                he_tri[he_map[(uint64_t(v1) << 32) | common[idx]]] = tri_pair;
            }
        }
        he_vert[v2_i] = v1;
    }
    for (int i = 0; i < vertices2.size(); i++)
    {
        int v2_i = he_map[(uint64_t(v2) << 32) | vertices2[i]];
        int v2_i_pair = he_pair[v2_i];
        he_map.erase((uint64_t(v2) << 32) | vertices2[i]);
        he_map.erase((uint64_t(vertices2[i]) << 32) | v2);
        he_map[(uint64_t(v1) << 32) | vertices2[i]] = v2_i;
        he_map[(uint64_t(vertices2[i]) << 32) | v1] = v2_i_pair;
    }
    vert_pos[v1] = (vert_pos[v1] + vert_pos[v2]) / 2.0f;

    std::vector<int> order_list = {he, pair};
    for (int i : common)
    {
        order_list.push_back(he_map[(uint64_t(v2) << 32) | i]);
        order_list.push_back(he_map[(uint64_t(i) << 32) | v2]);
        he_map.erase((uint64_t(v2) << 32) | i);
        he_map.erase((uint64_t(i) << 32) | v2);
    }
    he_map.erase((uint64_t(v1) << 32) | v2);
    he_map.erase((uint64_t(v2) << 32) | v1);
    std::sort(order_list.begin(), order_list.end());
    for (int i = order_list.size() - 1; i >= 0; i--)
    {
        delete_he(order_list[i]);
    }
    delete_tri(std::max(tri1, tri2));
    delete_tri(std::min(tri1, tri2));
    delete_vert(v2);
}

void HalfEdgeMesh::delete_tri(int tri)
{
    if (tri == -1)
    {
        return;
    }
    if (tri == n_tris - 1)
    {
        tri_he.pop_back();
        tri_verts.pop_back();
        n_tris--;
        return;
    }
    int he = tri_he.back();
    assert(he_tri[he] == n_tris - 1);
    he_tri[he] = tri;
    he = he_next[he];
    while (he != tri_he.back())
    {
        he_tri[he] = tri;
        he = he_next[he];
    }
    tri_he[tri] = tri_he.back();
    tri_verts[tri] = tri_verts.back();
    tri_he.pop_back();
    tri_verts.pop_back();
    n_tris--;
}

void HalfEdgeMesh::delete_he(int he)
{
    if (he == n_he - 1)
    {
        he_pair.pop_back();
        he_next.pop_back();
        he_tri.pop_back();
        he_vert.pop_back();
        n_he--;
        return;
    }
    int pair = he_pair.back();
    int v1 = he_vert.back();
    int v2 = he_vert[pair];
    int prev = he_prev(n_he - 1);
    vert_he[v1] = he;
    if (he_tri.back() != -1)
    {
        tri_he[he_tri.back()] = he;
    }
    he_next[prev] = he;
    he_next[he] = he_next.back();
    he_pair[pair] = he;
    he_pair[he] = pair;
    he_tri[he] = he_tri.back();
    he_vert[he] = v1;
    he_map[(uint64_t(v1) << 32) | v2] = he;
    he_vert.pop_back();
    he_next.pop_back();
    he_pair.pop_back();
    he_tri.pop_back();
    n_he--;
}

void HalfEdgeMesh::delete_vert(int vert)
{
    if (vert == n_verts - 1)
    {
        vert_he.pop_back();
        vert_normal.pop_back();
        vert_pos.pop_back();
        n_verts--;
        return;
    }
    vert_pos[vert] = vert_pos.back();
    vert_normal[vert] = vert_normal.back();
    vert_he[vert] = vert_he.back();
    std::vector<int> vertices = get_adjacent_vertices(n_verts - 1);
    for (int i = 0; i < vertices.size(); i++)
    {
        int he = he_map[(uint64_t(vertices[i]) << 32) | (n_verts - 1)];
        int pair = he_pair[he];
        int tri_1 = he_tri[he];
        int tri_2 = he_tri[pair];
        for (int idx = 0; idx < 3; idx++)
        {
            if (tri_1 != -1 && tri_verts[tri_1][idx] == n_verts - 1)
            {
                tri_verts[tri_1][idx] = vert;
            }
            if (tri_2 != -1 && tri_verts[tri_2][idx] == n_verts - 1)
            {
                tri_verts[tri_2][idx] = vert;
            }
        }
        he_vert[pair] = vert;
    }
    for (int i = 0; i < vertices.size(); i++)
    {
        int he = he_map[(uint64_t(vertices[i]) << 32) | (n_verts - 1)];
        int pair = he_pair[he];
        he_map.erase((uint64_t(vertices[i]) << 32) | (n_verts - 1));
        he_map.erase((uint64_t(n_verts - 1) << 32) | vertices[i]);
        he_map[(uint64_t(vertices[i]) << 32) | vert] = he;
        he_map[(uint64_t(vert) << 32) | vertices[i]] = pair;
    }
    vert_he.pop_back();
    vert_pos.pop_back();
    vert_normal.pop_back();
    n_verts--;
}

float HalfEdgeMesh::he_length(int he)
{
    int v1 = he_vert[he];
    int v2 = he_vert[he_pair[he]];
    return glm::length(vert_pos[v1] - vert_pos[v2]);
}

float cotan(float angle)
{
    return cos(angle) / sin(angle);
}

void HalfEdgeMesh::assign_weights()
{
    vert_gravity.clear();
    vert_gravity.resize(n_verts, glm::vec3(0.0, 0.0, 0.0));
    std::vector<float> vert_areas(n_verts, 0.0);
    // assign gravity to all vertices
    for (int q = 0; q < n_verts; q++)
    {
        std::vector<int> vertices = get_adjacent_vertices(q);
        for (int i = 0; i < vertices.size(); i++)
        {
            if (he_map.find((uint64_t(vertices[(i + 1) % vertices.size()]) << 32) | vertices[i]) != he_map.end())
            {
                int he = he_map[(uint64_t(vertices[(i + 1) % vertices.size()]) << 32) | vertices[i]];
                int tri = he_tri[he];
                if ((tri != -1) && (tri_verts[tri][0] == q || tri_verts[tri][1] == q || tri_verts[tri][2] == q))
                {
                    glm::vec3 q_one = vert_pos[vertices[i]] - vert_pos[q];
                    glm::vec3 one_two = vert_pos[vertices[(i + 1) % vertices.size()]] - vert_pos[vertices[i]];
                    glm::vec3 two_q = vert_pos[q] - vert_pos[vertices[(i + 1) % vertices.size()]];
                    float alpha_i = acos(glm::dot(q_one / glm::length(q_one), -two_q / glm::length(two_q)));
                    float alpha_j = acos(glm::dot(-q_one / glm::length(q_one), one_two / glm::length(one_two)));
                    float alpha_k = acos(glm::dot(two_q / glm::length(two_q), -one_two / glm::length(one_two)));
                    if (std::max({alpha_i, alpha_j, alpha_k}) <= M_PI / 2)
                    {
                        vert_areas[q] += (cotan(alpha_j) * glm::length(two_q) * glm::length(two_q) +
                                          cotan(alpha_k) * glm::length(q_one) * glm::length(q_one)) /
                                         8.0;
                    }
                    else if (alpha_i > M_PI / 2)
                    {
                        vert_areas[q] += 0.25 * glm::length(q_one) * glm::length(two_q) * sin(alpha_i);
                    }
                    else
                    {
                        vert_areas[q] += 0.125 * glm::length(q_one) * glm::length(two_q) * sin(alpha_i);
                    }
                }
            }
        }
    }

    for (int i = 0; i < n_verts; i++)
    {
        float denom = 0;
        std::vector<int> vertices = get_adjacent_vertices(i);
        for (int j = 0; j < vertices.size(); j++)
        {
            vert_gravity[i] += vert_areas[vertices[j]] * vert_pos[vertices[j]];
            denom += vert_areas[vertices[j]];
        }
        vert_gravity[i] /= denom;
    }
}

bool HalfEdgeMesh::flip_check(int he)
{
    int next = he_next[he];
    int prev = he_prev(he);
    int pair = he_pair[he];
    int pair_next = he_next[pair];
    int pair_prev = he_prev(pair);
    int origin = he_vert[he];
    int pair_origin = he_vert[pair];
    int new_origin = he_vert[pair_prev];
    int new_pair_origin = he_vert[prev];
    int tri = he_tri[he];
    int pair_tri = he_tri[pair];

    bool checks = true;
    checks &= (tri != -1);
    checks &= (pair_tri != -1);
    checks &= (he_tri[he_pair[next]] == -1) || (he_tri[he_pair[pair_prev]] == -1) ||
              (he_tri[he_pair[next]] != he_tri[he_pair[pair_prev]]);
    checks &= (he_tri[he_pair[prev]] == -1) || (he_tri[he_pair[pair_next]] == -1) ||
              (he_tri[he_pair[prev]] != he_tri[he_pair[pair_next]]);

    // geometric constraints
    std::vector<glm::vec3> vertices = {vert_pos[origin], vert_pos[new_origin], vert_pos[pair_origin],
                                       vert_pos[new_pair_origin]};
    std::vector<glm::vec3> crosses;
    for (int i = 0; i < 4; i++)
    {
        crosses.push_back(glm::cross(glm::normalize(vertices[(i + 2) % 4] - vertices[(i + 1) % 4]),
                                     glm::normalize(vertices[(i + 1) % 4] - vertices[i % 4])));
        checks &= (glm::length(crosses[i]) > 0.3); // approx 5 degrees

        for (int j = 0; j < i; j++)
        {
            checks &= (glm::dot(glm::normalize(crosses[j]), glm::normalize(crosses[i])) > 0.5);
        }
    }
    return checks;
}

bool HalfEdgeMesh::split_check(int he)
{
    if (he_tri[he_pair[he]] == -1 || he_tri[he] == -1)
    {
        return false;
    }
    return true;
}

void HalfEdgeMesh::remeshing(float l, float lambda)
{

    for (int i = 0; i < n_he; i++)
    {
        if (he_length(i) > (4 * l) / 3 && split_check(i))
        {
            edge_split(i);
        }
    }
    for (int i = 0; i < n_he; i++)
    {
        if (he_length(i) < (4 * l) / 5 && collapse_check(i) && collapse_check(he_pair[i]))
        {
            edge_collapse(i);
        }
    }
    recompute_vertex_normals();
    assign_weights();
    for (int i = 0; i < n_verts; i++)
    {
        vert_pos[i] = vert_pos[i] + lambda * (glm::mat3(1.0f) - glm::outerProduct(vert_normal[i], vert_normal[i])) *
                                        (vert_gravity[i] - vert_pos[i]);
    }
}

void HalfEdgeMesh::check_invariants()
{

    // check n_verts
    assert(n_verts == vert_pos.size());
    assert(n_verts == vert_normal.size());
    assert(n_verts == vert_he.size());
    // check n_he
    assert(n_he == he_map.size());
    assert(n_he == he_next.size());
    assert(n_he == he_pair.size());
    assert(n_he == he_tri.size());
    assert(n_he == he_vert.size());
    // check n_tris
    assert(n_tris == tri_he.size());
    assert(n_tris == tri_verts.size());

    // check for out of bounds errors
    assert(n_he - 1 >= *std::max_element(vert_he.begin(), vert_he.end()));
    assert(0 <= *std::min_element(vert_he.begin(), vert_he.end()));
    assert(n_he - 1 >= *std::max_element(tri_he.begin(), tri_he.end()));
    assert(0 <= *std::max_element(tri_he.begin(), tri_he.end()));
    for (int i = 0; i < n_tris; i++)
    {
        assert(n_verts - 1 >= std::max({tri_verts[i][0], tri_verts[i][1], tri_verts[i][2]}));
        assert(0 <= std::min({tri_verts[i][0], tri_verts[i][1], tri_verts[i][2]}));
    }
    assert(n_verts - 1 >= *std::max_element(he_vert.begin(), he_vert.end()));
    assert(0 <= *std::min_element(he_vert.begin(), he_vert.end()));
    assert(n_he - 1 >= *std::max_element(he_next.begin(), he_next.end()));
    assert(0 <= *std::min_element(he_next.begin(), he_next.end()));
    assert(n_he - 1 >= *std::max_element(he_pair.begin(), he_pair.end()));
    assert(0 <= *std::min_element(he_pair.begin(), he_pair.end()));
    assert(n_tris - 1 >= *std::max_element(he_tri.begin(), he_tri.end()));
    assert(-1 <= *std::min_element(he_tri.begin(), he_tri.end()));
    for (const auto &[v1v2, he] : he_map)
    {
        assert(n_verts - 1 >= (v1v2 >> 32));
        assert(0 <= (v1v2 >> 32));
        assert(n_verts - 1 >= (v1v2 & ((1UL << 32) - 1)));
        assert(0 <= (v1v2 & ((1UL << 32) - 1)));
    }
    for (const auto &[v1v2, he] : he_map)
    {
        assert(n_he - 1 >= he);
        assert(0 <= he);
    }
    // check if vert.he originates from vert
    for (int i = 0; i < n_verts; i++)
    {
        assert(he_vert[vert_he[i]] == i);
    }
    // check if tri.he is contained in tri
    for (int i = 0; i < n_tris; i++)
    {
        assert(he_tri[tri_he[i]] == i);
    }
    // check if tri.vs have edges in tri and tri.vs has unique vertices
    for (int i = 0; i < n_tris; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            assert(tri_verts[i][j] != tri_verts[i][(j + 1) % 3]);
            assert(he_tri[he_map[(uint64_t(tri_verts[i][j]) << 32) | tri_verts[i][(j + 1) % 3]]] == i);
        }
    }
    // check if he.source is correct
    for (const auto &[v1v2, he] : he_map)
    {
        assert(he_vert[he] == (v1v2 >> 32));
    }
    // check if he and he.next have same face and he.next != he
    for (int i = 0; i < n_he; i++)
    {
        assert(he_tri[i] == he_tri[he_next[i]]);
        assert(he_next[i] != i);
    }
    // check if he.target is same as he.next.source
    for (const auto &[v1v2, he] : he_map)
    {
        assert((v1v2 & ((1UL << 32) - 1)) == he_vert[he_next[he]]);
    }
    // check if he.pair.pair = he and he.pair != he
    for (int i = 0; i < n_he; i++)
    {
        assert(he_pair[he_pair[i]] == i);
        assert(he_pair[i] != i);
    }
    // check if he.target = he.pair.source. Checks for correct winding order. Also check if he.tri != he.pair.tri
    for (const auto &[v1v2, he] : he_map)
    {
        assert((v1v2 & ((1UL << 32) - 1)) == he_vert[he_pair[he]]);
        assert(he_tri[he] != he_tri[he_pair[he]]);
    }
    // check if all entries in he_map are unique
    std::unordered_set<int> he_map_set;
    for (const auto &[v1v2, he] : he_map)
    {
        assert(he_map_set.insert(he).second);
    }

    // each half edge is part of closed loop
    for (int i = 0; i < n_he; i++)
    {
        int he = i;
        int cnt = 0;
        while (he_next[he] != i)
        {
            he = he_next[he];
            cnt++;
            assert(cnt <= n_he + 20);
        }
    }
    // each interior edge is part of loop of size 3
    for (int i = 0; i < n_he; i++)
    {
        if (he_tri[i] == -1)
        {
            continue;
        }
        int cnt = 1;
        int he = he_next[i];
        while (he != i)
        {
            he = he_next[he];
            cnt++;
        }
        assert(cnt == 3);
    }

    // check if neighbours of triangle share at least 2 vertices with it
    for (int i = 0; i < n_tris; i++)
    {
        int start_he = tri_he[i];
        int he = start_he;
        do
        {
            int neighbour = he_tri[he_pair[he]];
            if (neighbour != -1)
            {
                int cnt = 0;
                for (int v1 = 0; v1 < 3; v1++)
                {
                    for (int v2 = 0; v2 < 3; v2++)
                    {
                        cnt += (tri_verts[i][v1] == tri_verts[neighbour][v2]);
                    }
                }
                assert(cnt == 2);
            }
            he = he_next[he];
        } while (he != start_he);
    }
}
