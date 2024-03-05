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
    int m = 20, n = 20; // m is slices(longi) n is stacks(lati)

    float latitude_dist = M_PI / n;
    float longitude_dist = 2 * M_PI / m;

    std::vector<glm::vec3> vert_pos;
    std::vector<glm::vec3> vert_normals;
    std::vector<glm::ivec3> faces;

    vert_pos.push_back(glm::vec3(0.0, 1.0, 0.0));
    vert_normals.push_back(vert_pos.back());
    for (int i = 1; i < n; i++)
    {
        float curr_latitude = i * latitude_dist;
        for (int j = 0; j < m; j++)
        {
            float curr_longitude = j * longitude_dist;
            vert_pos.push_back(glm::vec3(cosf(curr_longitude) * sinf(curr_latitude), cosf(curr_latitude),
                                         sinf(curr_longitude) * sinf(curr_latitude)));
            vert_normals.push_back(vert_pos.back());
        }
    }
    vert_pos.push_back(glm::vec3(0, -1.0, 0.0));
    vert_normals.push_back(vert_pos.back());

    for (int j = 0; j < m; j++)
    {
        // north pole and first layer
        faces.push_back(glm::ivec3(0, (j + 1) % m + 1, j + 1));
    }
    for (int i = 1; i < n - 1; i++)
    {
        for (int j = 0; j < m; j++)
        {
            // middle layers
            faces.push_back(glm::ivec3((i - 1) * m + j + 1, (i - 1) * m + (j + 1) % m + 1, i * m + j + 1));
            faces.push_back(glm::ivec3((i - 1) * m + (j + 1) % m + 1, i * m + (j + 1) % m + 1, i * m + j + 1));
        }
    }
    for (int j = 0; j < m; j++)
    {
        // south pole and last layer
        faces.push_back(glm::ivec3((n - 2) * m + j + 1, (n - 2) * m + (j + 1) % m + 1, n * m - m + 1));
    }

    mesh.set_vert_attribs(vert_pos, vert_normals);

    mesh.set_faces(faces);

    // mesh.recompute_vertex_normals();

    std::cout << "Loaded " << mesh.n_verts << " verts, " << mesh.n_tris << " triangles and " << mesh.n_he
              << "half edges\n";

    mesh.recompute_vertex_normals();
    mesh.check_invariants();
    // mesh.remeshing(l - 0.02, 0);

    float l = 0;
    for (int i = 0; i < mesh.n_he; i++)
    {
        l += mesh.he_length(i);
    }
    l /= mesh.n_he;
    std::cout << l << "\n";
    mesh.remeshing(l, 0.33);
    mesh.remeshing(l, 0.33);
    mesh.remeshing(l, 0.33);
    mesh.remeshing(l, 0.33);
    mesh.remeshing(l, 0.33);

    // mesh.remeshing(l - 0.02, 0.33);
    // mesh.remeshing(l - 0.02, 0.33);
    // mesh.remeshing(l - 0.02, 0.33);

    // mesh.taubin_smoothing(0.33, -0.33, 10);
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
