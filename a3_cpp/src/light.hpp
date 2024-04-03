#pragma once

#include <glm/glm.hpp>

struct LightSource {
    glm::vec3 pos, rgb;
    float intensity;
    
    LightSource(glm::vec3 _pos, glm::vec3 _rgb, float _intensity);
};
