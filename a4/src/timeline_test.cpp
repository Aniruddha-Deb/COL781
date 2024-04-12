#include "timeline.hpp"
#include <vector>
#include <iostream>

int main(int argc, char** argv) {

    Timeline timeline;

    std::vector<float> v1({0.f, 0.f});
    std::vector<float> v2({0.f, 1.f});
    std::vector<float> v3({1.f, 1.f});
    std::vector<float> v4({1.f, 0.f});
    glm::vec3 zero(0.f, 0.f, 0.f);

    timeline.add_frame({0.f, zero, v1});
    timeline.add_frame({1.f, zero, v2});
    timeline.add_frame({2.f, zero, v3});
    timeline.add_frame({3.f, zero, v4});

    for (int i=0; i<300; i++) {
        Keyframe k = timeline.interpolate_frame(float(i)/100);
        std::cout << k.bone_rotations[0] << "," << k.bone_rotations[1] << std::endl;
    }

    return 0;
}
