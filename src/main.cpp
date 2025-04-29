/**
This application displays a mesh in wireframe using "Modern" OpenGL 3.0+.
The Mesh3D class now initializes a "vertex array" on the GPU to store the vertices
	and faces of the mesh. To render, the Mesh3D object simply triggers the GPU to draw
	the stored mesh data.
We now transform local space vertices to clip space using uniform matrices in the vertex shader.
	See "simple_perspective.vert" for a vertex shader that uses uniform model, view, and projection
		matrices to transform to clip space.
	See "uniform_color.frag" for a fragment shader that sets a pixel to a uniform parameter.
*/
#define _USE_MATH_DEFINES
#include <glad/glad.h>
#include <iostream>
#include <memory>
#include <filesystem>
#include <optional>
#include <math.h>

#include "AssimpImport.h"
#include "Mesh3D.h"
#include "Object3D.h"
#include "Animator.h"
#include "ShaderProgram.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
// #include <SFML/Window/Event.hpp>
// #include <SFML/Window/Window.hpp>
// #include <SFML/Window/VideoMode.hpp>

#define SFML_V2

struct DirLight {
    glm::vec3 direction{ 1.0f, -1.0f, 0.0f };

    glm::vec3 ambient{ 0.1f, 0.1f, 0.1f };
    glm::vec3 diffuse{ 0.0f, 0.0f, 0.0f };
    glm::vec3 specular;
};

struct PointLight {
    glm::vec3 position;

    float constant = 1.0f;
    float linear = 0.14f;
    float quadratic = 0.07f;

    glm::vec3 ambient;
    glm::vec3 diffuse{ 0.4f, 0.4f, 0.4f };
    glm::vec3 specular;
};

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;

    float constant = 1.0f;
    float linear = 0.14f;
    float quadratic = 0.07f;

    glm::vec3 ambient{ 0.1f, 0.1f, 0.1f };
    glm::vec3 diffuse{ 0.5f, 0.0f, 0.5f };
    glm::vec3 specular{ 0.2f, 0.2f, 0.2f };
};

struct Scene {
    ShaderProgram program;
    std::vector<Object3D> objects;
    std::vector<Animator> animators;
    struct {
        glm::vec3 front;
        glm::vec3 position{ 0.0f, 0.0f, 5.0f };
        glm::vec3 orientation;

        glm::mat4 view;
        glm::mat4 perspective;
    } camera;

    DirLight dlight;
    PointLight plight;
    SpotLight slight;
};

/**
 * @brief Constructs a shader program that applies the Cell Shading model.
 */
ShaderProgram toonLightingShader() {
	ShaderProgram shader;
	try {
		shader.load("shaders/light_perspective.vert", "shaders/cell_lighting.frag");
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
		shader.load("shaders/light_perspective.vert", "shaders/lighting.frag");
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

/**
 * @brief Loads an image from the given path into an OpenGL texture.
 */
Texture loadTexture(const std::filesystem::path& path, const std::string& samplerName = "baseTexture") {
	StbImage i;
	i.loadFromFile(path.string());
	return Texture::loadImage(i, samplerName);
}

/*****************************************************************************************
*  DEMONSTRATION SCENES
*****************************************************************************************/
Scene bunny() {
	Scene scene{ texturingShader() };

	// We assume that (0,0) in texture space is the upper left corner, but some artists use (0,0) in the lower
	// left corner. In that case, we have to flip the V-coordinate of each UV texture location. The last parameter
	// to assimpLoad controls this. If you load a model and it looks very strange, try changing the last parameter.
	auto bunny = assimpLoad("models/bunny_textured.obj", true);
	bunny.grow(glm::vec3(9, 9, 9));
	bunny.move(glm::vec3(0.2, -1, 0));

	// Move all objects into the scene's objects list.
	scene.objects.push_back(std::move(bunny));
	// Now the "bunny" variable is empty; if we want to refer to the bunny object, we need to reference
	// scene.objects[0]

	Animator spinBunny;
	// Spin the bunny 360 degrees over 10 seconds.
	spinBunny.addAnimation(std::make_unique<RotationAnimation>(scene.objects[0], 10.0, glm::vec3(0, 2 * M_PI, 0)));

	// Move all animators into the scene's animators list.
	scene.animators.push_back(std::move(spinBunny));

	return scene;
}


/**
 * @brief Demonstrates loading a square, oriented as the "floor", with a manually-specified texture
 * that does not come from Assimp.
 */
Scene marbleSquare() {
	Scene scene{ texturingShader() };

	std::vector<Texture> textures = {
		loadTexture("models/White_marble_03/Textures_2K/white_marble_03_2k_baseColor.tga", "baseTexture"),
	};
	auto mesh = Mesh3D::square(textures);
	auto floor = Object3D(std::vector<Mesh3D>{mesh});
	floor.grow(glm::vec3(5, 5, 5));
	floor.move(glm::vec3(0, -1.5, 0));
	floor.rotate(glm::vec3(-M_PI / 2, 0, 0));

	scene.objects.push_back(std::move(floor));
	return scene;
}

/**
 * @brief Loads a cube with a cube map texture.
 */
Scene cube() {
	Scene scene{ phongLightingShader() };

	auto cube = assimpLoad("models/cube.obj", true);

	scene.objects.push_back(std::move(cube));

	Animator spinCube;
	spinCube.addAnimation(std::make_unique<RotationAnimation>(scene.objects[0], 10.0, glm::vec3(0, M_PI, 0)));
	// Then spin around the x axis.
	spinCube.addAnimation(std::make_unique<RotationAnimation>(scene.objects[0], 10.0, glm::vec3(M_PI, 0, 0)));

	scene.animators.push_back(std::move(spinCube));

	return scene;
}

Scene lightCube() {
    Scene scene { toonLightingShader() };

	auto cube = assimpLoad("models/cube.obj", true);

	scene.objects.push_back(std::move(cube));
	return scene;
}

/**
 * @brief Constructs a scene of a tiger sitting in a boat, where the tiger is the child object
 * of the boat.
 * @return
 */
Scene lifeOfPi() {
	// This scene is more complicated; it has child objects, as well as animators.
	Scene scene{ toonLightingShader() };

	auto boat = assimpLoad("models/boat/boat.fbx", true);
	boat.move(glm::vec3(0, -0.7, 0));
	boat.grow(glm::vec3(0.01, 0.01, 0.01));
	auto tiger = assimpLoad("models/tiger/scene.gltf", true);
	tiger.move(glm::vec3(0, -5, 10));
	// Move the tiger to be a child of the boat.
	boat.addChild(std::move(tiger));

	// Move the boat into the scene list.
	scene.objects.push_back(std::move(boat));

	// We want these animations to referenced the *moved* objects, which are no longer
	// in the variables named "tiger" and "boat". "boat" is now in the "objects" list at
	// index 0, and "tiger" is the index-1 child of the boat.
	Animator animBoat;
	animBoat.addAnimation(std::make_unique<RotationAnimation>(scene.objects[0], 10, glm::vec3(0, 2 * M_PI, 0)));
	Animator animTiger;
	animTiger.addAnimation(std::make_unique<RotationAnimation>(scene.objects[0].getChild(1), 10, glm::vec3(0, 2 * M_PI, 0)));

	// The Animators will be destroyed when leaving this function, so we move them into
	// a list to be returned.
	scene.animators.push_back(std::move(animBoat));
	scene.animators.push_back(std::move(animTiger));

	// Transfer ownership of the objects and animators back to the main.
	return scene;
}

Scene Rinoa() {
	Scene scene{ toonLightingShader() };

    scene.plight.position = {2.f, 2.f, -3.f};

	auto lady = assimpLoad("models/stickman/Simple_Character.fbx", true);
	lady.grow(glm::vec3(0.25));
	lady.move(glm::vec3(0, -25, -50));

	/*auto cube = assimpLoad("models/cube.obj", true);*/
	/*cube.move(glm::vec3(0.2, -1, -10));*/

	scene.objects.push_back(std::move(lady));

	Animator animLady;
	animLady.addAnimation(std::make_unique<RotationAnimation>(scene.objects[0], 10, glm::vec3(2 * M_PI, 2 * M_PI, 0)));

	scene.animators.push_back(std::move(animLady));

    return scene;
}


int main() {

	std::cout << std::filesystem::current_path() << std::endl;

	// Initialize the window and OpenGL.
	sf::ContextSettings settings;
	settings.depthBits = 24; // Request a 24 bits depth buffer
	settings.stencilBits = 8;  // Request a 8 bits stencil buffer
	settings.majorVersion = 3;
	settings.minorVersion = 3;
#ifdef SFML_V2
	settings.antialiasingLevel = 2;  // Request 2 levels of antialiasing
    sf::Window window(sf::VideoMode{ 1200, 800 }, "Modern OpenGL v2", sf::Style::Resize | sf::Style::Close, settings);
#else
	settings.antiAliasingLevel = 2;
	sf::Window window(sf::VideoMode({ 1200,800 }), "Modern OpenGL v3", sf::State::Windowed, settings);
#endif
	gladLoadGL();
	glEnable(GL_DEPTH_TEST);

	// Inintialize scene objects.
	auto myScene = Rinoa();
	// You can directly access specific objects in the scene using references.
	// auto& firstObject = myScene.objects[0];

	// Activate the shader program.
	myScene.program.activate();

	// Set up the view and projection matrices.

	// Ready, set, go!
	bool running = true;
	sf::Clock c;
	auto last = c.getElapsedTime();

    float& yaw = myScene.camera.orientation.x;
    float& pitch = myScene.camera.orientation.y;

	// Start the animators.
	for (auto& anim : myScene.animators) {
		anim.start();
	}

    // center the mouse initially.
    sf::Vector2<int> mousePosition = {};
    sf::Mouse::setPosition({(int)window.getSize().x / 2, (int)window.getSize().y / 2 }, window);
    bool firstMove = true;

	while (running) {
#ifdef SFML_V2
        sf::Event ev;
        while (window.pollEvent(ev)) {
			if (ev.type == sf::Event::Closed) {
				running = false;
			}
            else if (ev.type == sf::Event::Resized) {
                // window.setSize({ ev.size.width, ev.size.height });
                glViewport(0, 0, ev.size.width, ev.size.height);
            }
            else if (ev.type == sf::Event::KeyPressed) {
                switch (ev.key.code) {
                    case sf::Keyboard::Key::Escape: {
                        running = false;
                    } break;
                    default: {

                    } break;
                }
            }
            else if (ev.type == sf::Event::MouseMoved) {
                mousePosition.x = ev.mouseMove.x;
                mousePosition.y = ev.mouseMove.y;
            }
        }
#else
        while (const std::optional ev = window.pollEvent()) {
            if (ev->getIf<sf::Event::Closed>()) {
                running = false;
            }
            else if (const auto* resized = ev->getIf<sf::Event::Resized>()) {
                // window.setSize(resized->size);
                glViewport(0, 0, resized->size.x, resized->size.y);

            }
            else if (const auto* keyPressed = ev->getIf<sf::Event::KeyPressed>()) {
                switch (keyPressed->code) {
                    case sf::Keyboard::Key::Escape: {
                        running = false;
                    } break;
                    default: {

                    } break;
                }
            }
            else if (const auto* mouseMoved = ev->getIf<sf::Event::MouseMoved>()) {
                mousePosition = mouseMoved->position;
            }
		}
#endif
		auto now = c.getElapsedTime();
		auto diff = now - last;
		std::cout << 1 / diff.asSeconds() << " FPS " << std::endl;
		last = now;
        /*
         *  // logic for updating camera pos around a point
         *
         *  given theta  (angle), r (distance) representing polar coords;
         *  if (key A is pressed) {
         *      theta = ...;
         *      cameraPos = glm:vec3(r * cos(theta), ___, r * sin(theta));
         *  }
         *  given cameraDir (vector 3);
         *  camTarget = cameraPos + cameraTarget;
         *
         *  // rotate based on user input
         *  if (key RightArrow is pressed) {
         *      cameraDir = glm::rotate(cameraDir, glm::vec3(0,1,0), 0.001);
         *  }
         *
         *  // move camera based on user input
         *  if (key W is pressed) {
         *      cameraPos = cameraPos + cameraDir * 0.0001 (or dt);
         *  }
         *
         */
        int lastX;
        int lastY;

        if (firstMove) {
            lastX = mousePosition.x;
            lastY = mousePosition.y;
            yaw = -90.f;
            firstMove = false;
        }

        float offsetX = mousePosition.x - lastX;
        float offsetY = lastY - mousePosition.y;
        lastX = mousePosition.x;
        lastY = mousePosition.y;

        float sensitivity = 0.3f;
        offsetX *= sensitivity;
        offsetY *= sensitivity;


        yaw += offsetX; // yaw
        pitch += offsetY; // pitch

        if(pitch > 89.0f)
            pitch = 89.0f;
        if(pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        myScene.camera.front = glm::normalize(direction);

		myScene.camera.view = glm::lookAt(myScene.camera.position, myScene.camera.position + myScene.camera.front, glm::vec3(0, 1, 0));
		myScene.camera.perspective = glm::perspective(glm::radians(45.0), static_cast<double>(window.getSize().x) / window.getSize().y, 0.1, 100.0);

		myScene.program.setUniform("view", myScene.camera.view);
		myScene.program.setUniform("projection", myScene.camera.perspective);
		myScene.program.setUniform("viewPos", myScene.camera.position);

        // lighting
        myScene.program.setUniform("dirLight.direction", myScene.dlight.direction);
        myScene.program.setUniform("dirLight.ambient", myScene.dlight.ambient);
        myScene.program.setUniform("dirLight.diffuse", myScene.dlight.diffuse);
        myScene.program.setUniform("dirLight.specular", myScene.dlight.specular);

        myScene.program.setUniform("pointLight.position", myScene.plight.position);

        myScene.program.setUniform("pointLight.constant", myScene.plight.constant);
        myScene.program.setUniform("pointLight.linear", myScene.plight.linear);
        myScene.program.setUniform("pointLight.quadratic", myScene.plight.quadratic);

        myScene.program.setUniform("pointLight.ambient", myScene.plight.ambient);
        myScene.program.setUniform("pointLight.diffuse", myScene.plight.diffuse);
        myScene.program.setUniform("pointLight.specular", myScene.plight.specular);

        /*myScene.program.setUniform("spotLight.direction", myScene.slight.direction);*/
        /*myScene.program.setUniform("spotLight.position", myScene.slight.position);*/
        /*myScene.program.setUniform("spotLight.cutOff", myScene.slight.cutOff);*/
        myScene.program.setUniform("spotLight.direction", myScene.camera.front);
        myScene.program.setUniform("spotLight.position", myScene.camera.position);
        myScene.program.setUniform("spotLight.cutOff", glm::cos(glm::radians(12.5f)));

        myScene.program.setUniform("spotLight.constant", myScene.slight.constant);
        myScene.program.setUniform("spotLight.linear", myScene.slight.linear);
        myScene.program.setUniform("spotLight.quadratic", myScene.slight.quadratic);

        myScene.program.setUniform("spotLight.ambient", myScene.slight.ambient);
        myScene.program.setUniform("spotLight.diffuse", myScene.slight.diffuse);
        myScene.program.setUniform("spotLight.specular", myScene.slight.specular);

		// Update the scene.
		for (auto& anim : myScene.animators) {
			anim.tick(diff.asSeconds());
		}

		// Clear the OpenGL "context".
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Render the scene objects.
		for (auto& o : myScene.objects) {
			o.render(myScene.program);
		}
		window.display();

	}
    window.close();
	return 0;
}
