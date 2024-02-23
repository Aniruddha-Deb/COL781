#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include "glm/glm.hpp"

#include "mesh.hpp"

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
