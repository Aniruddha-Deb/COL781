#include "../src/a1.hpp"

namespace R = COL781::Software;
// namespace R = COL781::Hardware;
using namespace glm;

int main() {
	R::Rasterizer r;
    if (!r.initialize("Example 2", 640, 480))
        return EXIT_FAILURE;
    R::ShaderProgram program = r.createShaderProgram(
        r.vsColor(),
        r.fsIdentity()
    );
    vec4 vertices[] = {
		vec4( 0.0, -0.8, 0.0, 1.0),
        vec4( 0.8,  0.0, 0.0, 1.0),
        vec4( 0.0,  0.8, 0.0, 1.0),
        vec4(-0.8,  0.0, 0.0, 1.0)
    };
    vec4 colors[] = {
		vec4(0.0, 0.0, 0.0, 1.0),
        vec4(1.0, 0.0, 0.0, 1.0),
        vec4(1.0, 1.0, 1.0, 1.0),
        vec4(0.0, 0.0, 1.0, 1.0)
    };
	ivec3 triangles[] = {
		ivec3(0, 1, 2),
		ivec3(0, 2, 3)
	};
	R::Object shape = r.createObject();
	r.setVertexAttribs(shape, 0, 4, vertices);
	r.setVertexAttribs(shape, 1, 4, colors);
	r.setTriangleIndices(shape, 2, triangles);
    while (!r.shouldQuit()) {
        r.clear(vec4(1.0, 1.0, 1.0, 1.0));
        r.useShaderProgram(program);
		r.drawObject(shape);
        r.show();
    }
    r.deleteShaderProgram(program);
    return EXIT_SUCCESS;
}
