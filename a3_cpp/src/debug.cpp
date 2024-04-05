#include "debug.hpp"
#include <sstream>

std::string vec3_to_str(const glm::vec3& vec3) {
    std::stringstream ss;
    ss.precision(3);
    ss << "(" << vec3.x << ", " << vec3.y << ", " << vec3.z << ")";
    return ss.str();
}

void debug_hitrecord(HitRecord& rec) {

    cdebug << "HitRecord [\n"
           << "  ray: [\n"
           << "    o: " << vec3_to_str(rec.ray.o) << "\n"
           << "    d: " << vec3_to_str(rec.ray.d) << "\n"
           << "  ],\n"
           << "  pos: " << vec3_to_str(rec.pos) << "\n"
           << "  normal: " << vec3_to_str(rec.normal) << "\n"
           << "  n_bounces_left: " << rec.n_bounces_left << "\n"
           << "  t: " << rec.t << "\n"
           << "]" << std::endl;

}
