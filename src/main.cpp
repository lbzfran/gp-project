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

#include "Framebuffer.h"

#include "Scene.cpp"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

// #define SFML_V2

sf::Vector2<uint32_t> winSize = {1200, 800};

// uint32_t loadCubemap(vector<std::string> faces) {
//     unsigned int textureID;
//     glGenTextures(1, &textureID);
//     glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
//
//     int width, height, nrChannels;
//     for (unsigned int i = 0; i < faces.size(); i++) {
//         unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
//         if (data) {
//             glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
//                          0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
//             );
//             stbi_image_free(data);
//         }
//         else {
//             std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
//             stbi_image_free(data);
//         }
//     }
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//
//     return textureID;
// }

bool CheckCollision(Object3D& one, Object3D& two) {
    bool collidedX = one.getPosition().x + one.getScale().x >= two.getPosition().x && two.getPosition().x + two.getScale().x >= one.getPosition().x;
    bool collidedY = one.getPosition().y + one.getScale().y >= two.getPosition().y && two.getPosition().y + two.getScale().y >= one.getPosition().y;
    bool collidedZ = one.getPosition().z + one.getScale().z >= two.getPosition().z && two.getPosition().z + two.getScale().z >= one.getPosition().z;

    bool result = collidedX && collidedY && collidedZ;

    if (result)
        std::cout << "colliding!" << std::endl;
    else
        std::cout << "not colliding!" << std::endl;

    return result;
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
    sf::Window window(sf::VideoMode{ winSize.x, winSize.y }, "Modern OpenGL v2", sf::Style::Resize | sf::Style::Close, settings);
#else
	settings.antiAliasingLevel = 2;
	sf::Window window(sf::VideoMode({ winSize.x, winSize.y }), "Modern OpenGL v3", sf::State::Windowed, settings);
#endif
    winSize.x = window.getSize().x;
    winSize.y = window.getSize().y;
	gladLoadGL();

    // === GL GLOBAL SETS ===
    // DEPTH TEST
	glEnable(GL_DEPTH_TEST);

    // STENCIL TEST
    /*glEnable(GL_STENCIL_TEST);*/

    // FACE CULLING
	glEnable(GL_CULL_FACE);
    // NOTE(liam): could mess up models where front and back face must be visible
    glCullFace(GL_BACK);

	// Inintialize scene objects.
	auto myScene = Sanders();
	// You can directly access specific objects in the scene using references.
	// auto& firstObject = myScene.objects[0];

	// Activate the shader program.
	myScene.program.activate();

    ShaderProgram fbs = FB_simpleShader();
    fbs.activate();
    fbs.setUniform("screenTexture", 0);
    // Activate and initialize framebuffer's shader.
    // From now on, framebuffer will handle clearing and setting
    // the draw buffer.
    Framebuffer fb = Framebuffer(winSize.x, winSize.y, fbs, true);

	// Ready, set, go!
	bool running = true;
	sf::Clock c;
	auto last = c.getElapsedTime();

	// Start the animators.
	// for (auto& anim : myScene.animators) {
	// 	anim.start();
	// }

    float targetLockCooldown = 0.0;
    bool targetLock = false;

    bool isJumping = false;
    float jumpTimer = 0.0f;

    bool moveHeld = false;

    Object3D& floor = myScene.objects[0];
    Object3D& wall = myScene.objects[1];
    Object3D& player = myScene.objects[2];

    // center the mouse initially.
    sf::Vector2<int> centerPosition = {(int)winSize.x / 2, (int)winSize.y / 2};
    sf::Vector2<int> mousePosition = {};
    sf::Mouse::setPosition(centerPosition, window);

    bool lockCursor = true;
    window.setMouseCursorVisible(false);
    window.setMouseCursorGrabbed(true);

	while (running) {
#ifdef SFML_V2
        sf::Event ev;
        while (window.pollEvent(ev)) {
			if (ev.type == sf::Event::Closed) {
				running = false;
			}
            else if (ev.type == sf::Event::Resized) {
                winSize.x = static_cast<uint32_t>(ev.size.width);
                winSize.y = static_cast<uint32_t>(ev.size.height);

                centerPosition.x = static_cast<int>(winSize.x) / 2;
                centerPosition.y = static_cast<int>(winSize.y) / 2;

                window.setSize({ winSize.x, winSize.y });

                fb.Resize();

                myScene.camera.RequestPerspective();
            }
            else if (ev.type == sf::Event::MouseMoved) {
                mousePosition = sf::Mouse::getPosition(window);

                float mX = mousePosition.x - centerPosition.x;
                float mY = centerPosition.y - mousePosition.y;
                myScene.camera.ProcessMouseMove(mX, mY);
            }
            else if (ev.type == sf::Event::MouseWheelScrolled) {
                if (ev.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                    myScene.camera.ProcessMouseScroll(ev.mouseWheelScroll.delta);
                }
            }
        }
#else
        while (const std::optional ev = window.pollEvent()) {
            if (ev->getIf<sf::Event::Closed>()) {
                running = false;
            }
            else if (const auto* resized = ev->getIf<sf::Event::Resized>()) {
                winSize.x = static_cast<uint32_t>(resized->size.x);
                winSize.y = static_cast<uint32_t>(resized->size.y);

                centerPosition.x = static_cast<int>(winSize.x) / 2;
                centerPosition.y = static_cast<int>(winSize.y) / 2;

                window.setSize(winSize);

                fb.Resize();

                myScene.camera.RequestPerspective();
            }
            else if (ev->is<sf::Event::MouseMoved>()) {
                mousePosition = sf::Mouse::getPosition(window);

                float mX = mousePosition.x - centerPosition.x;
                float mY = centerPosition.y - mousePosition.y;
                myScene.camera.ProcessMouseMove(mX, mY);
            }
            else if (const auto* msScrolled = ev->getIf<sf::Event::MouseWheelScrolled>()) {
                if (msScrolled->wheel == sf::Mouse::Wheel::Vertical) {
                    myScene.camera.ProcessMouseScroll(msScrolled->delta);
                }
            }
		}
#endif
		auto now = c.getElapsedTime();
		auto diff = now - last;
        float dt = diff.asSeconds();
		// std::cout << 1 / diff.asSeconds() << " FPS " << std::endl;
		last = now;

        if (lockCursor) {
            sf::Mouse::setPosition(sf::Vector2<int>(winSize.x / 2, winSize.y / 2), window);
        }

        // === INPUT ===

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
            if (!targetLockCooldown) {
                if (!targetLock) {
                    // for (auto& anim : myScene.animators) {
                    //     anim.start();
                    // }
                    myScene.camera.SetTarget(&player.getPosition());
                }
                else {
                    myScene.camera.DropTarget();
                }

                targetLock = !targetLock;
                targetLockCooldown = 1.f;
            }
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape) && !lockCursor) {
            running = false;
        }


        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            if (!targetLock) {
                player.setPosition(myScene.camera.position);
            }
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
            if (!lockCursor) {
                lockCursor = !lockCursor;
                window.setMouseCursorGrabbed(true);
                window.setMouseCursorVisible(false);

                myScene.camera.ToggleFocus();
            }
            else {
                lockCursor = !lockCursor;
                window.setMouseCursorGrabbed(false);
                window.setMouseCursorVisible(true);

                myScene.camera.ToggleFocus();
            }
        }

        glm::vec3 totalAcceleration = glm::vec3(0);
        glm::vec3 totalRotAcceleration = glm::vec3(0);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::U)) {
            if (not isJumping && player.getPosition().y == 0.0) {
                isJumping = true;
                jumpTimer = 2.f;
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::O)) {
            for (auto& anim : myScene.animators) {
                anim.start();
            }
        }

        // if (player.getPosition().y > 0.0f)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::I)) {
                totalAcceleration += glm::vec3(0, 0, 5);
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K)) {
                totalAcceleration += glm::vec3(0, 0, -5);
            }
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J)) {
            totalAcceleration += glm::vec3(5, 0, 0);
            // totalRotAcceleration += glm::vec3(0, -2, 0);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::L)) {
            totalAcceleration += glm::vec3(-5, 0, 0);
            // totalRotAcceleration += glm::vec3(0, 2, 0);
        }

        glm::vec3 direction = glm::vec3(0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
            moveHeld = true;
            direction.x = 1.f;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            moveHeld = true;
            direction.x = -1.f;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
            moveHeld = true;
            direction.z = -1.f;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            moveHeld = true;
            direction.z = 1.f;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) {
            moveHeld = true;
            direction.y = -1.f;

        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) {
            moveHeld = true;
            direction.y = 1.f;
        }

        if (moveHeld) {
            myScene.camera.ProcessKeyboard(direction, dt);
            moveHeld = false;
            // keeps floor below player
            // myScene.objects[0].setPosition(glm::vec3(player.getPosition().x, 0, player.getPosition().z));
        }

        // === UPDATE ===

        // point light is attached to thung
        myScene.plight.position = myScene.objects[3].getPosition() + glm::vec3(0, 0.5f, 0);

        if (targetLockCooldown > 0.0) {
            targetLockCooldown -= dt;
            if (targetLockCooldown <= 0.0) {
                targetLockCooldown = 0.0;
            }
        }

        if (jumpTimer > 0.0) {
            jumpTimer -= dt;
            if (jumpTimer <= 0.0) {
                jumpTimer = 0.0;
                isJumping = false;
            }
        }

        if (isJumping) {
            totalAcceleration += glm::vec3(0, 5, 0);
        }

        player.setAcceleration(totalAcceleration);
        player.setRotAcceleration(totalRotAcceleration);

        myScene.camera.update((float)winSize.x, (float)winSize.y, dt);

		// Update the scene.
        // for (auto& o : myScene.objects) {
        for (int i = 0; i < myScene.objects.size(); i++) {
            auto& o = myScene.objects[i];

            // skip checking player and floor and wall
            if (&o == &player || &o == &floor || &o == &wall);
            else if (CheckCollision(player, o)) {
                // if (&o == &wall) {
                //     player.setVelocity(-player.getVelocity());
                // }
                // else
                {
                    myScene.objects.erase(myScene.objects.begin() + i);
                    player.grow(player.getScale() + glm::vec3(0.5));
                }
            }

            o.tick(dt);
        }

		for (auto& anim : myScene.animators) {
			anim.tick(dt);
		}

        // === RENDER ===
        // sends render calls to Texture map.
        // also clears the textures
        // and enables certain tests automatically
        fb.RenderOnTexture();
        // fb.RenderOnScreen();
        // fb.Clear();

        // sets what to do on fail; must be done every frame
        /*glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);*/
        // disables writing to stencil buffer by default
        /*glStencilMask(0x00);*/

        // render scene to texture buffer
        myScene.program.activate();

        /*glCullFace(GL_FRONT);*/
        GLSetCameraUniform(myScene);
		// Render the scene objects.
		for (auto& o : myScene.objects) {
			o.render(myScene.program);
		}
        /*glCullFace(GL_BACK);*/

        // enables writing to stencil buffer
        /*glStencilFunc(GL_ALWAYS, 1, 0xFF);*/
        /*glStencilMask(0xFF);*/

        // TODO(liam)
        // -- draw object

        // func makes sure to draw only on parts of container
        // not equal to 1 (0xFF).
        // Depth testing is also disabled to prevent 'borders'
        // from being overwritten by other objects (i.e, floor, etc.)
        /*glStencilFunc(GL_NOTEQUAL, 1, 0xFF);*/
        /*glStencilMask(0x00);*/
        /*glDisable(GL_DEPTH_TEST);*/

        // TODO(liam)
        // -- activate outline shader
        // -- draw scaled object

        /*glStencilMask(0xFF);*/
        /*glStencilFunc(GL_ALWAYS, 1, 0xFF);*/
        /*glEnable(GL_DEPTH_TEST);*/

        // sends texture map to view buffer.
        fb.TextureToScreen();

		window.display();
	}
    window.close();
	return 0;
}
