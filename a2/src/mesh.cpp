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
    vertices.push_back(he_vert[he_pair[he]]); // now it has a pair!
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

bool HalfEdgeMesh::v_in_tri(int tri, int vertex)
{
    return (tri != -1) && (tri_verts[tri][0] == vertex || tri_verts[tri][1] == vertex || tri_verts[tri][2] == vertex);
}

void HalfEdgeMesh::recompute_vertex_normals()
{

    // Liu (1999)
    // https://escholarship.org/content/qt7657d8h3/qt7657d8h3.pdf?t=ptt283
    vert_normal.resize(vert_pos.size());
    for (int q = 0; q < n_verts; q++)
    {
        std::vector<int> vertices = get_adjacent_vertices(q);
        // std::cout << "neighbours of " << q << " ";
        // for (int i : vertices)
        // {
        //     std::cout << i << " ";
        // }
        // std::cout << "\n";
        glm::vec3 normal(.0f, .0f, .0f);
        for (int i = 0; i < vertices.size(); i++)
        {
            if (he_map.find((uint64_t(vertices[(i + 1) % vertices.size()]) << 32) | vertices[i]) != he_map.end())
            {
                int he = he_map[(uint64_t(vertices[(i + 1) % vertices.size()]) << 32) | vertices[i]];
                if (v_in_tri(he_tri[he], q))
                {
                    // std::cout << "\n"
                    //           << q << " includes " << vertices[i] << " " << vertices[(i + 1) % vertices.size()] <<
                    //           "\n";
                    auto v1 = vert_pos[vertices[i]] - vert_pos[q];
                    float mv1 = length(v1);
                    auto v2 = vert_pos[vertices[(i + 1) % vertices.size()]] - vert_pos[q];
                    float mv2 = length(v2);
                    normal += glm::cross(v2, v1) / (mv1 * mv1 * mv2 * mv2);
                    // std::cout << (glm::cross(v2, v1) / (mv1 * mv1 * mv2 * mv2))[0] << " "
                    //           << (glm::cross(v2, v1) / (mv1 * mv1 * mv2 * mv2))[1] << " "
                    //           << (glm::cross(v2, v1) / (mv1 * mv1 * mv2 * mv2))[2] << "\n";
                }
            }
        }
        vert_normal[q] = glm::normalize(normal);
        // std::cout << "\n"
        //           << q << " done " << vert_normal[q][0] << " " << vert_normal[q][1] << " " << vert_normal[q][2] <<
        //           "\n";
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

void HalfEdgeMesh::edge_flip(int he)
{
    // check if edge is not boundary
    assert(he_tri[he] != -1 && he_tri[he_pair[he]] != -1);

    // get pointers
    int next = he_next[he];
    int prev = he_next[next];
    int pair = he_pair[he];
    int pair_next = he_next[pair];
    int pair_prev = he_next[pair_next];
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
    check_invariants();
}

void HalfEdgeMesh::edge_split(int he)
{
    // check if edge is not boundary
    assert(he_tri[he] != -1 && he_tri[he_pair[he]] != -1);

    // get pointers
    int next = he_next[he];
    int prev = he_next[next];
    int pair = he_pair[he];
    int pair_next = he_next[pair];
    int pair_prev = he_next[pair_next];
    int origin = he_vert[he];
    int pair_origin = he_vert[pair];
    int prev_origin = he_vert[prev];
    int pair_prev_origin = he_vert[pair_prev];
    int tri = he_tri[he];
    int pair_tri = he_tri[pair];

    // add new vertex and new edges while recycling old ones
    vert_pos.push_back((vert_pos[origin] + vert_pos[pair_origin]) / 2.0f);
    vert_normal.push_back(glm::vec3(0, 0, 0));
    vert_he.push_back(he);
    vert_he[origin] = n_he;
    vert_he[pair_origin] = pair;
    vert_he[prev_origin] = n_he + 4;
    vert_he[pair_prev_origin] = n_he + 2;
    tri_he[tri] = he;
    tri_he[pair_tri] = pair;
    tri_he.resize(n_tris + 2);
    tri_he[n_tris] = prev;
    tri_he[n_tris + 1] = pair_next;
    tri_verts.resize(n_tris + 2);
    tri_verts[tri] = glm::ivec3(n_verts, pair_origin, prev_origin);
    tri_verts[pair_tri] = glm::ivec3(n_verts, pair_prev_origin, pair_origin);
    tri_verts[n_tris] = glm::ivec3(n_verts, prev_origin, origin);
    tri_verts[n_tris + 1] = glm::ivec3(n_verts, origin, pair_prev_origin);
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
    he_next[he] = next;
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
    he_pair[n_he + 2] = n_he + 3;
    he_pair[n_he + 4] = n_he + 5;
    he_pair[n_he + 1] = n_he;
    he_pair[n_he + 3] = n_he + 2;
    he_pair[n_he + 5] = n_he + 4;
    he_tri.resize(n_he + 6);
    he_tri[he] = tri;
    he_tri[next] = tri;
    he_tri[n_he + 4] = tri;
    he_tri[pair] = pair_tri;
    he_tri[n_he + 3] = pair_tri;
    he_tri[pair_prev] = pair_tri;
    he_tri[prev] = n_tris;
    he_tri[n_he] = n_tris;
    he_tri[n_he + 5] = n_tris;
    he_tri[pair_next] = n_tris + 1;
    he_tri[n_he + 2] = n_tris + 1;
    he_tri[n_he + 1] = n_tris + 1;
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
    check_invariants();
}

void HalfEdgeMesh::edge_collapse(int he)
{

    // collapse one edge, eliminating two faces and adding one vertex in the
    // process
    // vertex's position is the midpoint of the collapsed edge.

    // TODO implement this for edge edges too (one neighbour)
    assert(he_tri[he] != -1 && he_tri[he_pair[he]] != -1);

    // get pointers
    int v1 = he_vert[he];
    int v2 = he_vert[he_pair[he]];
    // WLOG we collapse v2 and keep v1
    // mutual neighbours of both, as well as the halfedges to delete
    // top neighbour
    int n1 = he_vert[he_pair[he_next[he]]];
    int n2 = he_vert[he_pair[he_next[he_pair[he]]]];

    // v1->n2 edge reconnect
    int he_v1_n2 = he_map[edge(v1, n2)];
    int he_n1_v1 = he_map[edge(n1, v1)];

    int he_v2_n3 = he_next[he_pair[he_next[he]]];
    int he_n4_v2 = he_next[he_next[he_pair[he_next[he_next[he_pair[he]]]]]];
    // give up
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
        int he = he_next[i];
        int cnt = 0;
        while (he != i)
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

    // Manifold check
}
