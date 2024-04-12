#include "timeline.hpp"
#include <iostream>

float estimate_derivative(float q0, float q1, float q2, float t0, float t1, float t2) {
    float dt1 = t1 - t0;
    float dt2 = t2 - t1;

    float dq1 = q1 - q0;
    float dq2  = q2 - q1;

    return (dt1/(dt1 + dt2)) * (dq1/dt1 + dq2/dt2) + dq1/dt1;
}

glm::vec3 estimate_derivative(glm::vec3 q0, glm::vec3 q1, glm::vec3 q2, float t0, float t1, float t2) {
    float dt1 = t1 - t0;
    float dt2 = t2 - t1;

    glm::vec3 dq1 = q1 - q0;
    glm::vec3 dq2  = q2 - q1;

    return (dt1/(dt1 + dt2)) * (dq1/dt1 + dq2/dt2) + dq1/dt1;
}

float cubic_hermite(float u, float q0, float q1, float qp0, float qp1) {
    
    float u2 = u*u, u3 = u*u*u;
    float c1 = 2*u3 - 3*u2 + 1;
    float c2 = u3 - 2*u2 + u;
    float c3 = -2*u3 + 3*u2;
    float c4 = u3 - u2;

    return c1*q0 + c2*qp0 + c3*q1 + c4*qp1;
}

glm::vec3 cubic_hermite(float u, glm::vec3 q0, glm::vec3 q1, glm::vec3 qp0, glm::vec3 qp1) {
    
    float u2 = u*u, u3 = u*u*u;
    float c1 = 2*u3 - 3*u2 + 1;
    float c2 = u3 - 2*u2 + u;
    float c3 = -2*u3 + 3*u2;
    float c4 = u3 - u2;

    return c1*q0 + c2*qp0 + c3*q1 + c4*qp1;
}

Keyframe compute_frame_derivative(Keyframe& k0, Keyframe& k1, Keyframe& k2) {

    Keyframe derivative(k1);

    derivative.root_bone_pos = estimate_derivative(k0.root_bone_pos, k1.root_bone_pos, k2.root_bone_pos,
                                                   k0.time, k1.time, k2.time);

    for (int i=0; i<derivative.bone_rotations.size(); i++) {
        derivative.bone_rotations[i] = 
            estimate_derivative(k0.bone_rotations[i], k1.bone_rotations[i], k2.bone_rotations[i],
                                k0.time, k1.time, k2.time);
    }

    return derivative;
}

Keyframe interpolate_frame_catmull_rom(float time, Keyframe& k0, Keyframe& k1, Keyframe& kp0, Keyframe& kp1) {
    float u = (time - k0.time) / (k0.time - k1.time);

    Keyframe k;
    k.time = time;
    k.root_bone_pos = cubic_hermite(u, k0.root_bone_pos, k1.root_bone_pos, kp0.root_bone_pos, kp1.root_bone_pos);
    k.bone_rotations = std::vector<float>(k1.bone_rotations.size());
    for (int i=0; i<k.bone_rotations.size(); i++) {
        k.bone_rotations[i] = cubic_hermite(u, k0.bone_rotations[i], k1.bone_rotations[i], kp0.bone_rotations[i], kp1.bone_rotations[i]);
    }
    return k;
}

void Timeline::add_frame(Keyframe frame) {
    if (frames.size() == 0) {
        frames.push_back(frame);
        frame_derivatives.push_back(frame);
    }
    else if (frames.size() == 1) {
        frames.push_back(frame);
        frame_derivatives.push_back(frame);

        frame_derivatives[0].root_bone_pos = frames[1].root_bone_pos - frames[0].root_bone_pos;
        frame_derivatives[1].root_bone_pos = frames[1].root_bone_pos - frames[0].root_bone_pos;

        for (int i=0; i<frame.bone_rotations.size(); i++) {
            frame_derivatives[0].bone_rotations[i] = frame_derivatives[1].bone_rotations[i] = 
                frames[1].bone_rotations[i] - frames[0].bone_rotations[i];
        }
        // std::cout << "Added frame\n";
    }
    else {
        int n = frames.size();
        frames.push_back(frame);
        frame_derivatives.push_back(frame);

        frame_derivatives[n].root_bone_pos = frames[n-1].root_bone_pos - frames[n].root_bone_pos;

        for (int i=0; i<frame.bone_rotations.size(); i++) {
            frame_derivatives[n].bone_rotations[i] = frame_derivatives[1].bone_rotations[i] = 
                frames[n-1].bone_rotations[i] - frames[n-1].bone_rotations[i];
        }

        frame_derivatives[n-1] = compute_frame_derivative(frames[n-2], frames[n-1], frames[n]);
    }
}

Keyframe Timeline::interpolate_frame(float time) {

    // binary search for time. 
    // Assume time loops around if time > t_max
    time = fmod(time, frames[frames.size()-1].time);
    auto it = std::lower_bound(frames.begin(), frames.end(), time, [](const Keyframe& frame, const float t) {
            return frame.time <= t;
        });
    int idx = it - frames.begin();

    Keyframe frame = frames[idx];
    if (frames.size() > 1) {
        frame = interpolate_frame_catmull_rom(time, frames[idx-1], frames[idx], frame_derivatives[idx-1], frame_derivatives[idx]);
    }
    return frame;
}

void Timeline::request_frame(float time, std::vector<glm::vec3>& verts, std::vector<glm::ivec3>& idxs) {

    Keyframe frame = interpolate_frame(time);
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
