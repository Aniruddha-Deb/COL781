#include "hw.hpp"

#include <iostream>
#include <vector>

namespace COL781 {
	namespace Hardware {

		GLenum glCheckError_(const char *file, int line) {
			GLenum errorCode;
			while ((errorCode = glGetError()) != GL_NO_ERROR) {
				std::string error;
				switch (errorCode) {
				case GL_INVALID_ENUM:
					error = "INVALID_ENUM";
					break;
				case GL_INVALID_VALUE:
					error = "INVALID_VALUE";
					break;
				case GL_INVALID_OPERATION:
					error = "INVALID_OPERATION";
					break;
				case GL_STACK_OVERFLOW:
					error = "STACK_OVERFLOW";
					break;
				case GL_STACK_UNDERFLOW:
					error = "STACK_UNDERFLOW";
					break;
				case GL_OUT_OF_MEMORY:
					error = "OUT_OF_MEMORY";
					break;
				case GL_INVALID_FRAMEBUFFER_OPERATION:
					error = "INVALID_FRAMEBUFFER_OPERATION";
					break;
				}
				std::cout << error << " | " << file << " (" << line << ")" << std::endl;
			}
			return errorCode;
		}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

		bool Rasterizer::initialize(const std::string &title, int width, int height, int spp) {
			if (SDL_Init(SDL_INIT_VIDEO) < 0) {
				std::cout << "Could not initialize SDL: " << SDL_GetError() << std::endl;
				return false;
			}
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, spp);
			window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
			if (!window) {
				std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
				return false;
			}
			if (!SDL_GL_CreateContext(window)) {
				std::cerr << "Could not create OpenGL context: " << SDL_GetError() << std::endl;
				return false;
			}
			if (!gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress)) {
				std::cerr << "Failed to initialize GLAD" << std::endl;
				return false;
			}
			quit = false;
			glCheckError();
			return true;
		}

		bool Rasterizer::shouldQuit() {
			glCheckError();
			return quit;
		}

		ShaderProgram Rasterizer::createShaderProgram(const VertexShader &vs, const FragmentShader &fs) {
			ShaderProgram program = glCreateProgram();
			glAttachShader(program, vs);
			glAttachShader(program, fs);
			glLinkProgram(program);
			GLint linkStatus;
			glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
			if (linkStatus != GL_TRUE) {
				std::cout << "Error linking shaders:" << std::endl;
				GLint maxLength = 0;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(fs, maxLength, &maxLength, &infoLog[0]);
				std::cout << &infoLog[0] << std::endl;
				glDeleteProgram(program);
				glDeleteShader(vs);
				glDeleteShader(fs);
				return 0;
			}
			glCheckError();
			return program;
		}

		void Rasterizer::useShaderProgram(const ShaderProgram &program) {
			glUseProgram(program);
			glCheckError();
		}

		template <> void Rasterizer::setUniform(ShaderProgram &program, const std::string &name, float value) {
			GLint location = glGetUniformLocation(program, name.c_str());
			glUniform1f(location, value);
			glCheckError();
		}
		
		template <> void Rasterizer::setUniform(ShaderProgram &program, const std::string &name, int value) {
			GLint location = glGetUniformLocation(program, name.c_str());
			glUniform1i(location, value);
			glCheckError();
		}

		template <> void Rasterizer::setUniform(ShaderProgram &program, const std::string &name, glm::vec2 value) {
			GLint location = glGetUniformLocation(program, name.c_str());
			glUniform2fv(location, 1, &value[0]);
			glCheckError();
		}
		
		template <> void Rasterizer::setUniform(ShaderProgram &program, const std::string &name, glm::vec3 value) {
			GLint location = glGetUniformLocation(program, name.c_str());
			glUniform3fv(location, 1, &value[0]);
			glCheckError();
		}
		
		template <> void Rasterizer::setUniform(ShaderProgram &program, const std::string &name, glm::vec4 value) {
			GLint location = glGetUniformLocation(program, name.c_str());
			glUniform4fv(location, 1, &value[0]);
			glCheckError();
		}

		template <> void Rasterizer::setUniform(ShaderProgram &program, const std::string &name, glm::mat2 value) {
			GLint location = glGetUniformLocation(program, name.c_str());
			glUniformMatrix2fv(location, 1, GL_FALSE, &value[0][0]);
			glCheckError();
		}

		template <> void Rasterizer::setUniform(ShaderProgram &program, const std::string &name, glm::mat3 value) {
			GLint location = glGetUniformLocation(program, name.c_str());
			glUniformMatrix3fv(location, 1, GL_FALSE, &value[0][0]);
			glCheckError();
		}

		template <> void Rasterizer::setUniform(ShaderProgram &program, const std::string &name, glm::mat4 value) {
			GLint location = glGetUniformLocation(program, name.c_str());
			glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
			glCheckError();
		}

		void Rasterizer::deleteShaderProgram(ShaderProgram &program) {
			glDeleteProgram(program);
			glCheckError();
		}

		Object Rasterizer::createObject() {
			Object object;
			glGenVertexArrays(1, &object.vao);
			glCheckError();
			return object;
		}

		void setAttribs(Object &object, int attribIndex, int n, int d, const float* data) {
			GLuint vbo;
			glGenBuffers(1, &vbo);
			glBindVertexArray(object.vao);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, n*d*sizeof(float), data, GL_STATIC_DRAW);
			glVertexAttribPointer(attribIndex, d, GL_FLOAT, GL_FALSE, d*sizeof(float), NULL);
			glEnableVertexAttribArray(attribIndex);
			glCheckError();
		}

		template <> void Rasterizer::setVertexAttribs(Object &object, int attribIndex, int n, const float* data) {
			setAttribs(object, attribIndex, n, 1, data);
		}

		template <> void Rasterizer::setVertexAttribs(Object &object, int attribIndex, int n, const glm::vec2* data) {
			setAttribs(object, attribIndex, n, 2, (float*)data);
		}

		template <> void Rasterizer::setVertexAttribs(Object &object, int attribIndex, int n, const glm::vec3* data) {
			setAttribs(object, attribIndex, n, 3, (float*)data);
		}

		template <> void Rasterizer::setVertexAttribs(Object &object, int attribIndex, int n, const glm::vec4* data) {
			setAttribs(object, attribIndex, n, 4, (float*)data);
		}

		void Rasterizer::setTriangleIndices(Object &object, int n, glm::ivec3* indices) {
			GLuint ebo;
			glGenBuffers(1, &ebo);
			glBindVertexArray(object.vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*n*sizeof(int), (float*)indices, GL_STATIC_DRAW);
			object.nTris = n;
			glCheckError();
		}
		
		void Rasterizer::enableDepthTest() {
			glEnable(GL_DEPTH_TEST);
		    glDepthFunc(GL_LESS);   
			glCheckError();
		}

		void Rasterizer::clear(glm::vec4 color) {
			glClearColor(color[0], color[1], color[2], color[3]);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glCheckError();
		}

		// template <> Buffer<glm::vec4> Rasterizer::bufferVertexData(int n, glm::vec4* data){
		// 	GLuint VBO;
		// 	glGenBuffers(1, &VBO);
		// 	glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// 	glBufferData(GL_ARRAY_BUFFER, n*sizeof(glm::vec4), data, GL_STATIC_DRAW);
		// 	return VBO;
		// }

		// Buffer<glm::ivec3> Rasterizer::bufferElements(int n, glm::ivec3* data) {
		// 	GLuint IBO;
		// 	glGenBuffers(1, &IBO);
		// 	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		// 	glBufferData(GL_ELEMENT_ARRAY_BUFFER, n * sizeof(glm::ivec3), data, GL_STATIC_DRAW);
		// 	return IBO;
		// }

		// template <> void Rasterizer::setVertexAttribs<glm::vec4>(const ShaderProgram &program, const std::string &name, const Buffer<glm::vec4> buffer) {

		// 	GLint loc = glGetAttribLocation(program, name.c_str());

		// 	glEnableVertexAttribArray(loc);

		// 	glBindBuffer(GL_ARRAY_BUFFER, buffer);
		// 	glVertexAttribPointer(
		// 						  loc,
		// 						  4,
		// 						  GL_FLOAT,
		// 						  GL_FALSE,
		// 						  0, 
		// 						  (void *)0
		// 						  );

		// }

		void Rasterizer::drawObject(const Object &object) {
			glBindVertexArray(object.vao);
			glDrawElements(GL_TRIANGLES, 3*object.nTris, GL_UNSIGNED_INT, 0);
			glCheckError();
		}

		void Rasterizer::show() {
			SDL_GL_SwapWindow(window);
			SDL_Event e;
			while (SDL_PollEvent(&e) != 0) {
				if(e.type == SDL_QUIT) {
					quit = true;
				}
			}
			glCheckError();
		}

		GLuint createShader(GLenum type, const char *source) {
			GLuint shader = glCreateShader(type);
			glShaderSource(shader, 1, &source, NULL);
			glCompileShader(shader);
			GLint compileStatus;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
			if (compileStatus != GL_TRUE) {
				std::cout << "Error compiling vertex shader:" << std::endl;
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
				std::cout << &infoLog[0] << std::endl;
				glDeleteShader(shader);
				return 0;
			}
			glCheckError();
			return shader;
		}

		VertexShader Rasterizer::vsIdentity() {
			const char *source =
				"#version 330 core\n"
				"layout(location = 0) in vec4 vertex;\n"
				"void main() {\n"
				"	gl_Position = vertex;\n"
				"}\n";
			return createShader(GL_VERTEX_SHADER, source);
		}

		VertexShader Rasterizer::vsTransform() {
			const char *source =
				"#version 330 core\n"
				"layout(location = 0) in vec4 vertex;\n"
				"uniform mat4 transform;\n"
				"void main() {\n"
				"	gl_Position = transform * vertex;\n"
				"}\n";
			return createShader(GL_VERTEX_SHADER, source);
		}

		VertexShader Rasterizer::vsColor() {
			const char *source =
				"#version 330 core\n"
				"layout(location = 0) in vec4 vertex;\n"
				"layout(location = 1) in vec4 vColor;\n"
				"out vec4 color;\n"
				"void main() {\n"
				"	gl_Position = vertex;\n"
				"	color = vColor;\n"
				"}\n";
			return createShader(GL_VERTEX_SHADER, source);
		}
		
		VertexShader Rasterizer::vsColorTransform() {
			const char *source =
				"#version 330 core\n"
				"layout(location = 0) in vec4 vertex;\n"
				"layout(location = 1) in vec4 vColor;\n"
				"uniform mat4 transform;\n"
				"out vec4 color;\n"
				"void main() {\n"
				"	gl_Position = transform * vertex;\n"
				"	color = vColor;\n"
				"}\n";
			return createShader(GL_VERTEX_SHADER, source);
		}

		FragmentShader Rasterizer::fsConstant() {
			const char *source =
				"#version 330 core\n"  
				"uniform vec4 color;\n"
				"out vec4 fColor;\n"
				"void main() {\n"
				"	fColor = color;\n"
				"}\n";
			return createShader(GL_FRAGMENT_SHADER, source);
		}

		FragmentShader Rasterizer::fsIdentity() {
			const char *source =
				"#version 330 core\n"  
				"in vec4 color;\n"
				"out vec4 fColor;\n"
				"void main() {\n"
				"	fColor = color;\n"
				"}\n";
			return createShader(GL_FRAGMENT_SHADER, source);
		}

	}
}
