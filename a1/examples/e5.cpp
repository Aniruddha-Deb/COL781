#include "../src/a1.hpp"
#include <glm/gtc/matrix_transform.hpp>
// Program with perspective correct interpolation of vertex attributes.

// namespace R = COL781::Software;
namespace R = COL781::Hardware;
using namespace glm;
int main() {
	R::Rasterizer r;
	int width = 640, height = 480;
    if (!r.initialize("Example 5", width, height))
        return EXIT_FAILURE;

    R::ShaderProgram program = r.createShaderProgram(
        r.vsColorTransform(),
        r.fsIdentity()
    );
    vec4 vertices[] = {
        vec4( -0.8,  -0.8, 0.0, 1.0),
        vec4(  0.8,  -0.8, 0.0, 1.0),
		vec4( -0.8,   0.8, 0.0, 1.0),
        vec4(  0.8,   0.8, 0.0, 1.0)
    };
    vec4 colors[] = {
		vec4(0.0, 0.4, 0.6, 1.0),
        vec4(1.0, 1.0, 0.4, 1.0),
		vec4(0.0, 0.4, 0.6, 1.0),
        vec4(1.0, 1.0, 0.4, 1.0)
    };
	ivec3 triangles[] = {
		ivec3(0, 1, 2),
		ivec3(1, 2, 3)
	};
	R::Object shape = r.createObject();
	r.setVertexAttribs(shape, 0, 4, vertices);
	r.setVertexAttribs(shape, 1, 4, colors);
	r.setTriangleIndices(shape, 2, triangles);
    r.enableDepthTest();
    // The transformation matrix.
    mat4 model = mat4(1.0f);
	mat4 view = translate(mat4(1.0f), vec3(0.0f, 0.0f, -2.0f)); 
    mat4 projection = perspective(radians(60.0f), (float)width/(float)height, 0.1f, 100.0f);
    float speed = 90.0f; // degrees per second
    while (!r.shouldQuit()) {
        float time = SDL_GetTicks64()*1e-3;
        r.clear(vec4(1.0, 1.0, 1.0, 1.0));
        r.useShaderProgram(program);
        model = rotate(mat4(1.0f), radians(speed * time), vec3(1.0f,0.0f,0.0f));
        r.setUniform(program, "transform", projection * view * model);
		r.drawObject(shape);
        r.show();
    }
    r.deleteShaderProgram(program);
    return EXIT_SUCCESS;
}
