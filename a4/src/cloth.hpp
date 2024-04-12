#include <glm/glm.hpp>
#include <vector>
#include <set>

class Cloth
{
  public:
    float w, h;
    int res_w, res_h;
    float k_struct, k_shear, k_bend;
    float damp_factor;
    float mass;
    float time;
    std::vector<glm::vec3> vert_normals;
    std::vector<glm::vec3> vert_pos;
    std::vector<glm::vec3> vert_velocity;
    std::vector<glm::ivec3> faces;
    std::set<int> fixed;
    Cloth(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, int _res_w, int _res_h, float _k_struct,
          float _k_shear, float _k_bend, float _mass, float _time);

    void update(float t);
    void fix_vertex(int row, int col);
    glm::vec3 structural_force(int row, int col);
    glm::vec3 shear_force(int row, int col);
    glm::vec3 bending_force(int row, int col);
};