#include "material.hpp"
#include "constants.hpp"

glm::vec3 gamma_correct(glm::vec3 color)
{
    return glm::pow(color, glm::vec3(GAMMA, GAMMA, GAMMA));
}

glm::vec3 gamma_restore(glm::vec3 color)
{
    return glm::pow(color, glm::vec3(1.f / GAMMA, 1.f / GAMMA, 1.f / GAMMA));
}
