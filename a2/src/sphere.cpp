#include "viewer.hpp"
#include "mesh.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace V = COL781::Viewer;
using namespace glm;

template <typename T> void print_buffer(std::vector<T>& a, std::string bufname);

template <> void print_buffer(std::vector<int>& a, std::string bufname)
{

    std::cout << bufname << ": " << std::endl;
    for (int i : a)
    {
        std::cout << std::setfill(' ') << std::setw(2) << i << " ";
    }
    std::cout << std::endl;
}

template <> void print_buffer(std::vector<glm::vec3>& a, std::string bufname)
{

    std::cout << bufname << ": " << std::endl;
    for (auto i : a)
    {
        std::cout << std::setfill(' ') << std::setw(2) << "(" << i.x << ", " << i.y << ", " << i.z << ") ";
    }
    std::cout << std::endl;
}

int main(int argc, char** argv)
{
    V::Viewer v;
    if (!v.initialize("Mesh viewer", 640, 480))
    {
        return EXIT_FAILURE;
    }

    HalfEdgeMesh mesh;
    int m = 10, n = 10; // m is slices(longi) n is stacks(lati)

    float latitude_dist = M_PI / n;
    float longitude_dist = 2 * M_PI / m;

    std::vector<glm::vec3> vert_pos;
    std::vector<glm::vec3> vert_normals;
    std::vector<glm::ivec3> faces;

    for (int i = 0; i <= n; i++)
    {
        float curr_latitude = M_PI / 2 - i * latitude_dist;
        if (i == 0 || i == n)
        {
            vert_pos.push_back(
                glm::vec3(cosf(0.0) * cosf(curr_latitude), sinf(curr_latitude), sinf(0.0) * cosf(curr_latitude)));
            // vert_normals.push_back(vert_pos.back());
            continue;
        }
        for (int j = 0; j < m; j++)
        {
            float curr_longitude = j * longitude_dist;
            vert_pos.push_back(glm::vec3(cosf(curr_longitude) * cosf(curr_latitude), sinf(curr_latitude),
                                         sinf(curr_longitude) * cosf(curr_latitude)));
            // vert_normals.push_back(vert_pos.back());
        }
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            if (i == 0)
            {
                faces.push_back(glm::ivec3(0, (j + 1) % m + 1, j + 1));
            }
            else if (i == n - 1)
            {
                faces.push_back(glm::ivec3((i - 1) * m + j + 1, (i - 1) * m + (j + 1) % m + 1, n * m - m + 1));
            }
            else
            {
                faces.push_back(glm::ivec3((i - 1) * m + j + 1, (i - 1) * m + (j + 1) % m + 1, i * m + j + 1));
                faces.push_back(glm::ivec3((i - 1) * m + (j + 1) % m + 1, i * m + (j + 1) % m + 1, i * m + j + 1));
            }
        }
    }

    mesh.set_vert_attribs(vert_pos, vert_normals);

    mesh.set_faces(faces);

    mesh.recompute_vertex_normals();

    std::cout << "Loaded " << mesh.n_verts << " verts, " << mesh.n_tris << " triangles and " << mesh.n_he
              << "half edges\n";

    print_buffer(mesh.he_next, "he_next");
    print_buffer(mesh.he_pair, "he_pair");
    print_buffer(mesh.he_vert, "he_vert");
    // print_buffer(mesh.vert_pos, "vert_pos");
    // print_buffer(mesh.vert_normal, "vert_normal");

    // mesh.recompute_vertex_normals();

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
