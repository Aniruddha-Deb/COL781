#pragma once

#include <glm/glm.hpp>

struct LightSource {
    glm::vec3 pos, rgb;
    
    LightSource(glm::vec3 _pos, glm::vec3 _rgb);
};
