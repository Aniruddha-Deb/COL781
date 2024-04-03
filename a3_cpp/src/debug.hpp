#pragma once

#include <iostream>
#include "ray.hpp"

#ifdef DEBUG
#define cdebug std::cerr
#else
#define cdebug 0 && std::cerr
#endif

void debug_hitrecord(HitRecord& rec);
std::string vec3_to_str(glm::vec3 vec3);
