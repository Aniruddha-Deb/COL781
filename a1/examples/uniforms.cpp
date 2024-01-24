#include "../src/a1.hpp"
#include <iostream>

namespace R = COL781::Software;
// namespace R = COL781::Hardware;
using namespace glm;

int main() {
	R::Rasterizer r;
    if (!r.initialize("Example 1", 640, 480, 4))
        return EXIT_FAILURE;

    auto sp = r.createShaderProgram(
        r.vsIdentity(),
        r.fsIdentity()
    );

    /*
    R::Attribs a;
    a.set<float>(0, 1.0f);
    std::cout << a.get<float>(0) << std::endl;
    */

    /*
    R::Uniforms u;
    u.set<int>("color", 5);
    std::cout << u.get<int>("color") << std::endl;
    */

    sp.uniforms.set<int>("color", 5);
    std::cout << sp.uniforms.get<int>("color") << std::endl;

    return EXIT_SUCCESS;
}
