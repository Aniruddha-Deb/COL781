#ifndef HW_HPP
#define HW_HPP

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <string>

namespace COL781 {
	namespace Hardware {

		using VertexShader = GLuint;
		using FragmentShader = GLuint;

		using ShaderProgram = GLuint;

		struct Object {
			GLuint vao;
			int nTris;
		};

		class Rasterizer {
		public:
#include "api.inc"
		private:
			SDL_Window *window;
			bool quit;
		};

	}
}

#endif
