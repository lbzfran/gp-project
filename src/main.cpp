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

#define CAMERA_SPEED_MAX 0.2f
#define CAMERA_SPEED_ACCEL 0.1f
#define CAMERA_SPEED_DECAY 0.4f

struct DirLight {
    bool display = true;

    glm::vec3 direction{ 1.0f, -1.0f, -1.0f };

    glm::vec3 ambient{ 0.2f, 0.2f, 0.2f };
    glm::vec3 diffuse{ 0.4f, 0.4f, 0.4f };
    glm::vec3 specular{ 0.2f, 0.2f, 0.2f };
};

struct PointLight {
    bool display = true;

    glm::vec3 position;

    float constant = 1.0f;
    float linear = 0.14f;
    float quadratic = 0.07f;

    glm::vec3 ambient{ 0.1f, 0.1f, 0.1f };
    glm::vec3 diffuse{ 0.3f, 0.4f, 0.4f };
    glm::vec3 specular{ 0.4f, 0.4f, 0.4f };
};

struct SpotLight {
    bool display = true;

    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;

    float constant = 1.0f;
    float linear = 0.14f;
    float quadratic = 0.07f;

    glm::vec3 ambient{ 0.1f, 0.1f, 0.1f };
    glm::vec3 diffuse{ 0.8f, 0.8f, 0.8f };
    glm::vec3 specular{ 0.4f, 0.4f, 0.4f };
};

struct Camera {
    glm::vec3 front;
    glm::vec3 position{ 0.0f, 0.0f, 5.0f };
    glm::vec3 orientation;
    glm::vec3 up{ 0.0f, 1.0f, 0.0f };

    // front, up, and side speed
    glm::vec3 speed{ 0.f, 0.f, 0.f };

    glm::mat4 view;
    glm::mat4 perspective;


    bool requestUpdate = true;
};

struct Scene {
    ShaderProgram program;
    std::vector<Object3D> objects;
    std::vector<Animator> animators;

    Camera camera;

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
		shader.load("shaders/light_perspective.vert", "shaders/gl_cell_lighting.frag");
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
	Scene scene{ toonLightingShader() };

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

float Lerp(float a, float t, float b) {
    return a * (1 - t) + b * t;
}

glm::vec3 GLVec3Lerp(glm::vec3 a, float t, glm::vec3 b) {
    return glm::vec3(
        Lerp(a.x, t, b.x),
        Lerp(a.y, t, b.y),
        Lerp(a.z, t, b.z)
    );
}

void UpdateCamera(Camera& camera, sf::Vector2<int> mousePosition, bool updateMouse = true, glm::vec3 target = glm::vec3(0)) {
    static int lastX;
    static int lastY;
    static bool firstMove = true;
    static float sensitivity = 0.3f;

    glm::vec3 direction;

    if (updateMouse) {
        float offsetX = mousePosition.x - lastX;
        float offsetY = lastY - mousePosition.y;

        float& yaw = camera.orientation.x;
        float& pitch = camera.orientation.y;

        if (firstMove) {
            lastX = mousePosition.x;
            lastY = mousePosition.y;
            yaw = -90.f;
            firstMove = false;
        }

        lastX = mousePosition.x;
        lastY = mousePosition.y;

        offsetX *= sensitivity;
        offsetY *= sensitivity;

        yaw += offsetX; // yaw
        pitch += offsetY; // pitch

        if(pitch > 89.0f)
            pitch = 89.0f;
        if(pitch < -89.0f)
            pitch = -89.0f;

        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        camera.front = glm::normalize(direction);
    }

    camera.view = glm::lookAt(camera.position, target, camera.up);
}

void GLSetCameraUniform(Scene& scene) {
    Camera& camera = scene.camera;
    ShaderProgram& program = scene.program;

    program.setUniform("view", camera.view);
    program.setUniform("projection", camera.perspective);
    program.setUniform("viewPos", camera.position);

    /// lighting
    // directional light
    if (scene.dlight.display) {
        program.setUniform("dirLight.direction", scene.dlight.direction);
        program.setUniform("dirLight.ambient", scene.dlight.ambient);
        program.setUniform("dirLight.diffuse", scene.dlight.diffuse);
        program.setUniform("dirLight.specular", scene.dlight.specular);
    }
    else {
        program.setUniform("dirLight.ambient", glm::vec3(0));
        program.setUniform("dirLight.diffuse", glm::vec3(0));
        program.setUniform("dirLight.specular", glm::vec3(0));
    }

    // point light
    if (scene.plight.display) {
        program.setUniform("pointLight.position", scene.plight.position);

        program.setUniform("pointLight.constant", scene.plight.constant);
        program.setUniform("pointLight.linear", scene.plight.linear);
        program.setUniform("pointLight.quadratic", scene.plight.quadratic);

        program.setUniform("pointLight.ambient", scene.plight.ambient);
        program.setUniform("pointLight.diffuse", scene.plight.diffuse);
        program.setUniform("pointLight.specular", scene.plight.specular);
    }
    else {
        program.setUniform("pointLight.ambient", glm::vec3(0));
        program.setUniform("pointLight.diffuse", glm::vec3(0));
        program.setUniform("pointLight.specular", glm::vec3(0));
    }

    // spotlight
    if (scene.slight.display) {
        program.setUniform("spotLight.direction", scene.slight.direction);
        program.setUniform("spotLight.position", scene.slight.position);
        program.setUniform("spotLight.cutOff", scene.slight.cutOff);

        // flashlight
        /*program.setUniform("spotLight.direction", camera.front);*/
        /*program.setUniform("spotLight.position", camera.position);*/
        /*program.setUniform("spotLight.cutOff", glm::cos(glm::radians(25.f)));*/

        program.setUniform("spotLight.constant", scene.slight.constant);
        program.setUniform("spotLight.linear", scene.slight.linear);
        program.setUniform("spotLight.quadratic", scene.slight.quadratic);

        program.setUniform("spotLight.ambient", scene.slight.ambient);
        program.setUniform("spotLight.diffuse", scene.slight.diffuse);
        program.setUniform("spotLight.specular", scene.slight.specular);
    }
    else {
        program.setUniform("spotLight.ambient", glm::vec3(0));
        program.setUniform("spotLight.diffuse", glm::vec3(0));
        program.setUniform("spotLight.specular", glm::vec3(0));
    }
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
	sf::Window window(sf::VideoMode({ 1200, 800 }), "Modern OpenGL v3", sf::State::Windowed, settings);
#endif
	gladLoadGL();
	glEnable(GL_DEPTH_TEST);

	// Inintialize scene objects.
	auto myScene = marbleSquare();
	// You can directly access specific objects in the scene using references.
	// auto& firstObject = myScene.objects[0];

	// Activate the shader program.
	myScene.program.activate();

	// Set up the view and projection matrices.

	// Ready, set, go!
	bool running = true;
	sf::Clock c;
    sf::Clock deltaClock;
	auto last = c.getElapsedTime();

    float dt = 0.f;

    // myScene.camera.view = glm::lookAt(myScene.camera.position, myScene.camera.position + myScene.camera.front, myScene.camera.up);

	// Start the animators.
	for (auto& anim : myScene.animators) {
		anim.start();
	}

    //
    int moveDirUp = 0;
    int moveDirFront = 0;
    int moveDirSide = 0;

    float targetLockCooldown = 0.0;
    bool targetLock = false;
    glm::vec3 targetCamPos = glm::vec3(0);
    bool isMoving = false;
    bool moveHeld = false;

    float lerpedTargetLock = 0.0f;

    // TODO: lerp target position with camera position
    float lerpedTargetPos = 0.0f;

    // center the mouse initially.
    sf::Vector2<int> mousePosition = {(int)window.getSize().x / 2, (int)window.getSize().y / 2};
    sf::Mouse::setPosition(mousePosition, window);
    window.setMouseCursorVisible(false);
    window.setMouseCursorGrabbed(true);
    // window.setKeyRepeatEnabled(true);

    // initialize camera
    myScene.camera.perspective = glm::perspective(glm::radians(45.0), static_cast<double>(window.getSize().x) / window.getSize().y, 0.1, 100.0);

	while (running) {
        dt = deltaClock.restart().asSeconds();
#ifdef SFML_V2
        sf::Event ev;
        while (window.pollEvent(ev)) {
			if (ev.type == sf::Event::Closed || (ev.type == sf::Event::KeyPressed && ev.key.code == sf::Keyboard::Key::Escape)) {
				running = false;
			}
            else if (ev.type == sf::Event::Resized) {
                window.setSize({ ev.size.width, ev.size.height });
                glViewport(0, 0, ev.size.width, ev.size.height);
            }
            else if (ev.type == sf::Event::MouseMoved) {
                mousePosition = sf::Mouse::getPosition(window);
                myScene.camera.requestUpdate = true;
            }
        }
#else
        while (const std::optional ev = window.pollEvent()) {
            if (ev->getIf<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
                running = false;
            }
            else if (const auto* resized = ev->getIf<sf::Event::Resized>()) {
                window.setSize(resized->size);
                glViewport(0, 0, resized->size.x, resized->size.y);
                myScene.camera.perspective = glm::perspective(glm::radians(45.0), static_cast<double>(window.getSize().x) / window.getSize().y, 0.1, 100.0);
            }
            else if (ev->is<sf::Event::MouseMoved>()) {
                mousePosition = sf::Mouse::getPosition(window);
                myScene.camera.requestUpdate = true;
            }
		}
#endif
		auto now = c.getElapsedTime();
		auto diff = now - last;
		// std::cout << 1 / diff.asSeconds() << " FPS " << std::endl;
		last = now;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
            if (!targetLockCooldown) {
                targetLock = !targetLock;

                std::swap(targetCamPos, myScene.camera.position);

                targetLockCooldown = 1.f;
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
            if (moveDirFront == -1) {
                myScene.camera.speed.x = 0.f;
            }
            moveHeld = true;
            moveDirFront = 1;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            if (moveDirFront == 1) {
                myScene.camera.speed.x = 0.f;
            }
            moveHeld = true;
            moveDirFront = -1;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
            if (moveDirSide == 1) {
                myScene.camera.speed.z = 0.f;
            }
            moveHeld = true;
            moveDirSide = -1;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            if (moveDirSide == -1) {
                myScene.camera.speed.z = 0.f;
            }
            moveHeld = true;
            moveDirSide = 1;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) {
            if (moveDirUp == 1) {
                myScene.camera.speed.y = 0.f;
            }
            moveHeld = true;
            moveDirUp = -1;

        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {
            if (moveDirUp == -1) {
                myScene.camera.speed.y = 0.f;
            }
            moveHeld = true;
            moveDirUp = 1;

        }


        if (moveHeld) {
            if (moveDirFront) {
                myScene.camera.speed.x += CAMERA_SPEED_ACCEL * dt;
                if (myScene.camera.speed.x >= CAMERA_SPEED_MAX) {
                    myScene.camera.speed.x = CAMERA_SPEED_MAX;
                }
            }
            if (moveDirSide) {
                myScene.camera.speed.z += CAMERA_SPEED_ACCEL * dt;
                if (myScene.camera.speed.z >= CAMERA_SPEED_MAX) {
                    myScene.camera.speed.z = CAMERA_SPEED_MAX;
                }
            }
            if (moveDirUp) {
                myScene.camera.speed.y += CAMERA_SPEED_ACCEL * dt;
                if (myScene.camera.speed.y >= CAMERA_SPEED_MAX) {
                    myScene.camera.speed.y = CAMERA_SPEED_MAX;
                }
            }

            moveHeld = false;
            isMoving = true;
            myScene.camera.requestUpdate = true;
        }
        else if (isMoving) {
            std::cout << "speed: x = " << myScene.camera.speed.x << " y = " << myScene.camera.speed.y << " z = " << myScene.camera.speed.z << std::endl;
            if (myScene.camera.speed.x + myScene.camera.speed.y + myScene.camera.speed.z) {

                if (myScene.camera.speed.x) {
                    myScene.camera.speed.x -= CAMERA_SPEED_DECAY * dt;

                    if (myScene.camera.speed.x <= 0.0f) {
                        myScene.camera.speed.x = 0.0f;

                        moveDirFront = 0;
                    }
                }
                if (myScene.camera.speed.y) {
                    myScene.camera.speed.y -= CAMERA_SPEED_DECAY * dt;

                    if (myScene.camera.speed.y <= 0.0f) {
                        myScene.camera.speed.y = 0.0f;

                        moveDirSide = 0;
                    }
                }
                if (myScene.camera.speed.z) {
                    myScene.camera.speed.z -= CAMERA_SPEED_DECAY * dt;

                    if (myScene.camera.speed.z <= 0.0f) {
                        myScene.camera.speed.z = 0.0f;

                        moveDirUp = 0;
                    }
                }
                myScene.camera.requestUpdate = true;
            }
            else {
                isMoving = false;
            }
        }

        if (moveDirFront == 1) {
            myScene.camera.position += myScene.camera.speed.x * myScene.camera.front;
        }
        else if (moveDirFront == -1) {
            myScene.camera.position -= myScene.camera.speed.x * myScene.camera.front;
        }

        if (moveDirUp == 1) { // y movement
            myScene.camera.position += myScene.camera.speed.y * myScene.camera.up;
        }
        else if (moveDirUp == -1) {
            myScene.camera.position -= myScene.camera.speed.y * myScene.camera.up;
        }

        if (moveDirSide == -1) {
            myScene.camera.position -= glm::normalize(glm::cross(myScene.camera.front, myScene.camera.up)) * myScene.camera.speed.z;
        }
        else if (moveDirSide == 1) {
            myScene.camera.position += glm::normalize(glm::cross(myScene.camera.front, myScene.camera.up)) * myScene.camera.speed.z;
        }

        if (targetLock && lerpedTargetLock < 1.0) {
            std::cout << "track lerp: " << lerpedTargetLock << std::endl;
            myScene.camera.requestUpdate = true;
            lerpedTargetLock += 2.0 * dt;
            if (lerpedTargetLock >= 1.0) {
                lerpedTargetLock = 1.0f;
            }
        }
        else if (!targetLock && lerpedTargetLock > 0.0) {
            std::cout << "track lerp: " << lerpedTargetLock << std::endl;
            myScene.camera.requestUpdate = true;
            lerpedTargetLock -= 2.0 * dt;
            if (lerpedTargetLock <= 0.0) {
                lerpedTargetLock = 0.0f;
            }
        }

        if (targetLockCooldown > 0.0) {
            targetLockCooldown -= dt;
            if (targetLockCooldown <= 0.0) {
                targetLockCooldown = 0.0;
            }
        }

        if (!targetLock) {
            targetCamPos = myScene.camera.position;
        }



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

        glm::vec3 target = GLVec3Lerp(myScene.camera.position + myScene.camera.front, lerpedTargetLock, myScene.objects[0].getPosition());

        if (myScene.camera.requestUpdate) {
            UpdateCamera(myScene.camera, mousePosition, !targetLock, target);
            myScene.camera.requestUpdate = false;
        }
        GLSetCameraUniform(myScene);

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
