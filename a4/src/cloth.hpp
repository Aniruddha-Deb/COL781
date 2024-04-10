#include <glm/glm.hpp>
#include <vector>

class Cloth
{
  public:
    int w, h;
    int res_w, res_h;
    float k_struct, k_shear, k_bend;
    float mass;
    std::vector<glm::vec3> vert_pos;
    Cloth(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, int _res_w, int _res_h, float _k_struct,
          float _k_shear, float _k_bend, float _mass);

    void update(float t);
};