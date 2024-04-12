#include "bone.hpp"

Bone::Bone(std::shared_ptr<Bone> _parent, glm::vec3 _attachment_pos, glm::vec3 _rot_axis):
    parent{_parent}, attachment_pos{_attachment_pos}, rot_axis{_rot_axis}, rot_quat{1.f, 0.f, 0.f, 0.f} {}

void Bone::set_angle(float angle_rad) {
    rot_quat = glm::quat(glm::cos(angle_rad/2), rot_axis*glm::sin(angle_rad/2));
}

void Bone::set_position(glm::vec3 direction) {
    attachment_pos = direction;
}

glm::quat Bone::world_rot_quat() {
    if (parent) return parent->world_rot_quat() * rot_quat;
    return rot_quat;
}

glm::vec3 quat_to_vec3(glm::quat q) {
    return glm::vec3(q.x, q.y, q.z);
}

glm::vec3 rotate_pt(glm::vec3 pt, glm::quat q) {
    return quat_to_vec3(q * glm::quat(0.f, pt) * glm::conjugate(q));
}

glm::vec3 Bone::world_attachment_pos() {
    if (parent) return parent->world_attachment_pos() + rotate_pt(attachment_pos, parent->world_rot_quat());
    return attachment_pos;
}

glm::mat4x4 Bone::world_transformation_mat() {
    glm::mat4x4 transform_mat = glm::mat4_cast(world_rot_quat());
    transform_mat[3] = glm::vec4(world_attachment_pos(), 1.f);
    return transform_mat;
}
