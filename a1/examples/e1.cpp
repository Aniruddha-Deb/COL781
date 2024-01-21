#include "../src/a1.hpp"

// namespace R = COL781::Software;
namespace R = COL781::Hardware;
using namespace glm;

int main() {
	R::Rasterizer r;
    if (!r.initialize("Example 1", 640, 480))
        return EXIT_FAILURE;
    R::ShaderProgram program = r.createShaderProgram(
        r.vsIdentity(),
        r.fsConstant()
    );
    vec4 vertices[] = {
		vec4(-0.8,  0.0, 0.0, 1.0),
        vec4(-0.4, -0.8, 0.0, 1.0),
        vec4( 0.8,  0.8, 0.0, 1.0),
        vec4(-0.4, -0.4, 0.0, 1.0)
    };
	ivec3 triangles[] = {
		ivec3(0, 1, 3),
		ivec3(1, 2, 3)
	};
	R::Object tickmark = r.createObject();
	r.setVertexAttribs(tickmark, 0, 4, vertices);
	r.setTriangleIndices(tickmark, 2, triangles);
    while (!r.shouldQuit()) {
        r.clear(vec4(1.0, 1.0, 1.0, 1.0));
        r.useShaderProgram(program);
        r.setUniform<vec4>(program, "color", vec4(0.0, 0.6, 0.0, 1.0));
		r.drawObject(tickmark);
        r.show();
    }
    r.deleteShaderProgram(program);
    return EXIT_SUCCESS;
}
