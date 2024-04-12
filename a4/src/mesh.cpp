#include "mesh.hpp"

BoxMesh::BoxMesh(glm::vec3 min_pos, glm::vec3 max_pos) {
    verts.push_back(glm::vec3(min_pos.x, min_pos.y, min_pos.z));
    verts.push_back(glm::vec3(min_pos.x, min_pos.y, max_pos.z));
    verts.push_back(glm::vec3(min_pos.x, max_pos.y, min_pos.z));
    verts.push_back(glm::vec3(min_pos.x, max_pos.y, max_pos.z));
    verts.push_back(glm::vec3(max_pos.x, min_pos.y, min_pos.z));
    verts.push_back(glm::vec3(max_pos.x, min_pos.y, max_pos.z));
    verts.push_back(glm::vec3(max_pos.x, max_pos.y, min_pos.z));
    verts.push_back(glm::vec3(max_pos.x, max_pos.y, max_pos.z));

    idxs.push_back(glm::ivec3(0, 1, 3));
    idxs.push_back(glm::ivec3(0, 3, 2));
    idxs.push_back(glm::ivec3(2, 3, 7));
    idxs.push_back(glm::ivec3(2, 7, 6));
    idxs.push_back(glm::ivec3(6, 7, 5));
    idxs.push_back(glm::ivec3(6, 5, 4));
    idxs.push_back(glm::ivec3(4, 5, 1));
    idxs.push_back(glm::ivec3(4, 1, 0));
    idxs.push_back(glm::ivec3(1, 7, 3));
    idxs.push_back(glm::ivec3(1, 5, 7));
    idxs.push_back(glm::ivec3(0, 6, 4));
    idxs.push_back(glm::ivec3(0, 2, 6));
}
