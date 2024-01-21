#include "../src/a1.hpp"
#include <glm/gtc/matrix_transform.hpp>
// namespace R = COL781::Software;
namespace R = COL781::Hardware;
using namespace glm;

int main() {
	R::Rasterizer r;
    if (!r.initialize("Example 1", 640, 480))
        return EXIT_FAILURE;
    R::ShaderProgram program = r.createShaderProgram(
        r.vsTransform(),
        r.fsConstant()
    );
    vec4 vertices[] = {
		vec4(-0.5, -0.5, 0.0, 1.0),
        vec4(0.25, 0.35, 0.2, 1.0),
        vec4(-0.25, 0.5, 0.0, 1.0),
    };
	ivec3 triangles[] = {
	ivec3(0, 1, 2)
	};
    
    R::Object box = r.createObject();
	r.setVertexAttribs(box, 0, 3, vertices);
	r.setTriangleIndices(box, 1, triangles);
    
    // Enable depth test.
    r.enableDepthTest();

    // The transformation matrix.
    mat4 mvp = mat4(1.0f);
    while (!r.shouldQuit()) {
        r.clear(vec4(1.0, 1.0, 1.0, 1.0));
        r.useShaderProgram(program);

        mvp = mat4(1.0f);
        r.setUniform(program, "transform", mvp);
        r.setUniform(program, "color", vec4(0.9, 0.6, 0.3, 1.0));
		r.drawObject(box);

        mvp = mat4(1.0f);
        mvp = rotate(mvp, radians(120.0f), normalize(vec3(0.0f, 0.0f, 1.0f)));
        r.setUniform(program, "transform", mvp);
        r.setUniform(program, "color", vec4(0.8, 0.8, 0.8, 1.0));
		r.drawObject(box);

        mvp = mat4(1.0f);
        mvp = rotate(mvp, radians(-120.0f), normalize(vec3(0.0f, 0.0f, 1.0f)));
        r.setUniform(program, "transform", mvp);
        r.setUniform(program, "color", vec4(0.5, 0.5, 0.8, 1.0));
		r.drawObject(box);

        r.show();
    }
    r.deleteShaderProgram(program);
    return EXIT_SUCCESS;
}
