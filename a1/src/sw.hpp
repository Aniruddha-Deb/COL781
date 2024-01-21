#ifndef SW_HPP
#define SW_HPP

#include <glm/glm.hpp>
#include <map>
#include <SDL2/SDL.h>
#include <string>
#include <vector>

namespace COL781 {
	namespace Software {

		class Attribs {
			// A class to contain the attributes of ONE vertex
		public:
			// only float, glm::vec2, glm::vec3, glm::vec4 allowed
			template <typename T> T get(int attribIndex) const;
			template <typename T> void set(int attribIndex, T value);
		private:
			std::vector<glm::vec4> values;
			std::vector<int> dims;
		};

		class Uniforms {
			// A class to contain all the uniform variables
		public:
			// any type allowed
			template <typename T> T get(const std::string &name) const;
			template <typename T> void set(const std::string &name, T value);
		private:
			std::map<std::string,void*> values;
		};

		/* A vertex shader is a function that:
		   reads the uniform variables and one vertex's input attributes,
		   writes the output attributes for interpolation to fragments,
		   and returns the vertex position in NDC as a homogeneous vec4. */
		using VertexShader = glm::vec4(*)(const Uniforms &uniforms, const Attribs &in, Attribs &out);

		/* A fragment shader is a function that:
		   reads the uniform variables and one fragment's interpolated attributes,
		   and returns the colour of the fragment as an RGBA value. */
		using FragmentShader = glm::vec4(*)(const Uniforms &uniforms, const Attribs &in);

		struct ShaderProgram {
			VertexShader vs;
			FragmentShader fs;
			Uniforms uniforms;
		};

		struct Object {
			using Buffer = std::vector<float>;
			std::vector<Buffer> attributeValues;
			std::vector<int> attributeDims;
			std::vector<glm::ivec3> indices;
		};

#include "api.inc"

	}
}

#endif
