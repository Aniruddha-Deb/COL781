#ifndef HW_HPP
#define HW_HPP

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <string>

namespace COL781 {
	namespace OpenGL {

		using VertexShader = GLuint;
		using FragmentShader = GLuint;

		using ShaderProgram = GLuint;

		struct Object {
			GLuint vao;
			int nTris;
		};

		using AttribBuf = GLuint;
		using IndexBuf = GLuint;

		class Rasterizer {
		public:

			/** Windows **/

			// Creates a window with the given title, size, and samples per pixel.
			bool initialize(const std::string &title, int width, int height, int spp=1);

			// Returns true if the user has requested to quit the program.
			bool shouldQuit(); 

			/** Shader programs **/

			// Creates a new shader program, i.e. a pair of a vertex shader and a fragment shader.
			ShaderProgram createShaderProgram(const VertexShader &vs, const FragmentShader &fs);

			// Makes the given shader program active. Future draw calls will use its vertex and fragment shaders.
			void useShaderProgram(const ShaderProgram &program);

			// Sets the value of a uniform variable.
			// T is only allowed to be float, int, glm::vec2/3/4, glm::mat2/3/4.
			template <typename T> void setUniform(ShaderProgram &program, const std::string &name, T value);

			// Deletes the given shader program.
			void deleteShaderProgram(ShaderProgram &program);

			/** Objects **/

			// Creates an object, i.e. a collection of vertices and triangles.
			// Vertex attribute arrays store the vertex data.
			// A triangle index array stores the indices of the triangles.
			Object createObject();

			// Creates a buffer for the i'th vertex attribute.
			// T is only allowed to be float, glm::vec2, glm::vec3, or glm::vec4.
			template <typename T> AttribBuf createVertexAttribs(Object &object, int attribIndex, int n, const T* data);

			// Updates the data for the given attribute buffer.
			// T is only allowed to be float, glm::vec2, glm::vec3, or glm::vec4.
			template <typename T> void updateVertexAttribs(AttribBuf &buf, int n, const T* data);

			// Creates a buffer for the indices of the triangles.
			IndexBuf createTriangleIndices(Object &object, int n, const glm::ivec3* indices);

			/** Drawing **/

			// Enable depth testing.
			void enableDepthTest();

			// Clear the framebuffer, setting all pixels to the given color.
			void clear(glm::vec4 color);

			// Draws the triangles of the given object.
			void drawObject(const Object &object);

			// Draws only the edges of the polygon mesh. Note that it offsets the edges to avoid z-buffer fighting.
			void setupWireFrame();

			// Draws the faces of the polygon mesh.
			void setupFilledFaces();

			// Displays the framebuffer on the screen.
			void show(); 

			// glm::vec3 getCameraUpdate(float cameraSpeed); 

			/** Built-in shaders **/

            // A vertex shader that supports the Blinn-Phong shading model.
            VertexShader vsBlinnPhong();

            // A fragment shader that supports the Blinn-Phong shading model.
            FragmentShader fsBlinnPhong();

		private:
			SDL_Window *window;
			bool quit;
		};

	}
}

#endif
