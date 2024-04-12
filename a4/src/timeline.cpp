#include "timeline.hpp"
#include <iostream>

void Timeline::request_frame(float time, std::vector<glm::vec3>& verts, std::vector<glm::ivec3>& idxs) {

    Keyframe frame = frames[0];
    // TODO catmull-rom interp

    model[0].second->set_position(frame.root_bone_pos);

    for (int i=0; i<model.size(); i++) {
        model[i].second->set_angle(frame.bone_rotations[i]);
    }

    for (const auto& [mesh, bone] : model) {
        int start_idx = verts.size();
        for (const glm::vec3& vert : mesh->verts) {
            glm::vec4 tv_hom(vert, 1.f);
            tv_hom = bone->world_transformation_mat() * tv_hom;
            verts.push_back(glm::vec3(tv_hom)/tv_hom[3]);
        }
        for (const glm::ivec3& idx : mesh->idxs) {
            idxs.push_back(start_idx + idx);
        }
    }

}
