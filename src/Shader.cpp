#include "ShaderProgram.h"

ShaderProgram toonLightingShader() {
	ShaderProgram shader;
	try {
		shader.load("shaders/light_perspective.vert", "shaders/gl_cell_lighting.frag");
	}
	catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
	return shader;
}

ShaderProgram FB_simpleShader() {
    ShaderProgram shader;
    try {
        shader.load("shaders/post_process/fb_simple.vert", "shaders/post_process/fb_simple.frag");
    }
    catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
        exit(1);
    }
    return shader;
}

ShaderProgram FB_sharpenShader() {
    ShaderProgram shader;
    try {
        shader.load("shaders/post_process/fb_simple.vert", "shaders/post_process/fb_sharpen.frag");
    }
    catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
        exit(1);
    }
    return shader;
}

/**
 * @brief Constructs a shader program that applies the Phong reflection model.
 */
ShaderProgram phongLightingShader() {
	ShaderProgram shader;
	try {
		shader.load("shaders/light_perspective.vert", "shaders/gl_phong_lighting.frag");
	}
	catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
	return shader;
}

/**
 * @brief Constructs a shader program that performs texture mapping with no lighting.
 */
ShaderProgram texturingShader() {
	ShaderProgram shader;
	try {
		shader.load("shaders/texture_perspective.vert", "shaders/texturing.frag");
	}
	catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
	return shader;
}

ShaderProgram simpleShader() {
	ShaderProgram shader;
	try {
		shader.load("shaders/simple_perspective.vert", "shaders/uniform_color.frag");
	}
	catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
	return shader;
}
