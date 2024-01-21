// Do not include this file directly!
// The contents will be included into the files a1sw.hpp and a1hw.hpp

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

	// Sets the data for the i'th vertex attribute.
	// T is only allowed to be float, glm::vec2, glm::vec3, or glm::vec4.
	template <typename T> void setVertexAttribs(Object &object, int attribIndex, int n, const T* data);

	// Sets the indices of the triangles.
	void setTriangleIndices(Object &object, int n, glm::ivec3* indices);

	/** Drawing **/
	

	// Enable depth testing.
	void enableDepthTest();

	// Clear the framebuffer, setting all pixels to the given color.
	void clear(glm::vec4 color);

	// Draws the triangles of the given object.
	void drawObject(const Object &object);

	// Displays the framebuffer on the screen.
	void show(); 

	/** Built-in shaders **/

	// A vertex shader that uses the 0th vertex attribute as the position.
	VertexShader vsIdentity();

	// A vertex shader that uses the 0th vertex attribute as the position and passes on the 1th attribute as the color.
	VertexShader vsColor();

	// A vertex shader that applies the transformation matrix given by the uniform named 'transform'.
	VertexShader vsTransform();

	// A vertex shader that handles both transformation and color attributes.
	VertexShader vsColorTransform();

	// A fragment shader that returns a constant colour given by the uniform named 'color'.
	FragmentShader fsConstant(); 

	// A fragment shader that uses the 0th attribute as the color.
	FragmentShader fsIdentity(); 

private:
	SDL_Window *window;
	bool quit;
};
