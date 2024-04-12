#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "mesh.hpp"
#include "bone.hpp"

struct Keyframe {

    float time;
    glm::vec3 root_bone_pos;
    std::vector<float> bone_rotations;
};

class Timeline {

    public:
    std::vector<Keyframe> frames;
    std::vector<std::pair<std::shared_ptr<Mesh>, std::shared_ptr<Bone>>> model;

    void request_frame(float time, std::vector<glm::vec3>& verts, std::vector<glm::ivec3>& idxs);
};
