#include "../src/a1.hpp"
#include <glm/gtc/matrix_transform.hpp>
namespace R = COL781::Software;
// namespace R = COL781::Hardware;
using namespace glm;

int main() {
	R::Rasterizer r;
    if (!r.initialize("Example 1", 640, 480))
        return EXIT_FAILURE;
    R::ShaderProgram program = r.createShaderProgram(
        r.vsIdentity(),
        r.fsConstant()
    );
	R::Object tri1 = r.createObject();
	{
		vec4 vertices[] = {
			vec4(-0.25, -0.4, 0, 1.0),
			vec4(0.25, -0.4, 0, 1.0),
			vec4(0.0, 0.8, 0.1, 1.0),
		};
		ivec3 triangles[] = {
			ivec3(0, 1, 2)
		};
		r.setVertexAttribs(tri1, 0, 3, vertices);
		r.setTriangleIndices(tri1, 1, triangles);
	}
	R::Object tri2 = r.createObject();
	{
		vec4 vertices[] = {
			vec4(0.4, 0.4, 0, 1.0),
			vec4(-0.4, 0.4, 0, 1.0),
			vec4(0.0, -0.6, 0.2, 1.0),
		};
		ivec3 triangles[] = {
			ivec3(0, 1, 2)
		};
		r.setVertexAttribs(tri2, 0, 3, vertices);
		r.setTriangleIndices(tri2, 1, triangles);
	}
    
    // Enable depth test.
    r.enableDepthTest();

    while (!r.shouldQuit()) {
        r.clear(vec4(1.0, 1.0, 1.0, 1.0));
        r.useShaderProgram(program);

        r.setUniform<vec4>(program, "color", vec4(0.9, 0.7, 0.5, 1.0));
		r.drawObject(tri1);

        r.setUniform<vec4>(program, "color", vec4(0.6, 0.6, 0.8, 1.0));
		r.drawObject(tri2);

        r.show();
    }
    r.deleteShaderProgram(program);
    return EXIT_SUCCESS;
}
