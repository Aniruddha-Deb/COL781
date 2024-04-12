#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Bone {

    std::shared_ptr<Bone> parent;
    glm::vec3 attachment_pos;
    glm::vec3 rot_axis;

    glm::quat rot_quat;

    public:
    Bone(std::shared_ptr<Bone> _parent, glm::vec3 _attachment_pos, glm::vec3 _rot_axis);
    void set_angle(float angle_rad);
    void set_position(glm::vec3 position);

    glm::vec3 world_attachment_pos();
    glm::quat world_rot_quat();
    glm::mat4x4 world_transformation_mat();
};
