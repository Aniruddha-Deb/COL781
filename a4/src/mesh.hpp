#pragma once

#include <vector>
#include <glm/glm.hpp>

class Mesh {

    public:
    std::vector<glm::vec3> verts;
    std::vector<glm::ivec3> idxs;
};

class BoxMesh : public Mesh {

    public:
    BoxMesh(glm::vec3 min_pos, glm::vec3 max_pos);

};
