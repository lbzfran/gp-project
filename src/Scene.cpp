
#include "AssimpImport.h"
#include "Mesh3D.h"
#include "Object3D.h"
#include "Camera.h"

#include "Animator.h"
#include "RotationAnimation.h"
#include "TranslationAnimation.h"
#include "PauseAnimation.h"
#include "BezierTranslationAnimation.h"

#include "Shader.cpp"

struct DirLight {
    bool display = true;

    glm::vec3 direction{ 1.0f, 1.0f, 0.0f };

    glm::vec3 ambient{ 0.2f, 0.2f, 0.2f };
    glm::vec3 diffuse{ 0.6f, 0.6f, 0.6f };
    glm::vec3 specular{ 0.4f, 0.4f, 0.4f };
};

struct PointLight {
    bool display = true;

    glm::vec3 position{ 2.f, 2.0f, 0.f };

    float constant = 1.0f;
    float linear = 0.14f;
    float quadratic = 0.07f;

    glm::vec3 ambient{ 0.2f, 0.2f, 0.2f };
    glm::vec3 diffuse{ 0.8f, 0.8f, 0.0f };
    glm::vec3 specular{ 0.1f, 0.1f, 0.1f };
};

struct SpotLight {
    bool display = true;

    glm::vec3 position{ 0.f, 1.f, 0.f };
    glm::vec3 direction{ 0.f, -1.f, 0.f };
    float cutOff = 12.5f;
    float outerCutOff = 15.0f;

    float constant = 1.0f;
    float linear = 0.35f;
    float quadratic = 0.44f;

    glm::vec3 ambient{ 0.0f, 0.0f, 0.0f };
    glm::vec3 diffuse{ 0.0f, 0.0f, 0.4f };
    glm::vec3 specular{ 0.8f, 0.8f, 0.8f };
};

struct Scene {
    ShaderProgram program;
    std::vector<Object3D> objects;
    std::vector<Animator> animators;

    Camera camera = Camera();

    DirLight dlight;
    PointLight plight;
    SpotLight slight;
};

void GLSetCameraUniform(Scene& scene) {
    Camera& camera = scene.camera;
    ShaderProgram& program = scene.program;

    program.setUniform("view", camera.view);
    program.setUniform("projection", camera.perspective);
    program.setUniform("viewPos", camera.position);

    program.setUniform("ambientColor", glm::vec3(0.1f));

    /// lighting
    // directional light
    program.setUniform("dirLight.direction", scene.dlight.direction);
    program.setUniform("dirLight.ambient", scene.dlight.ambient);
    program.setUniform("dirLight.diffuse", scene.dlight.diffuse);
    program.setUniform("dirLight.specular", scene.dlight.specular);

    // point light
    program.setUniform("pointLight.position", scene.plight.position);

    program.setUniform("pointLight.constant", scene.plight.constant);
    program.setUniform("pointLight.linear", scene.plight.linear);
    program.setUniform("pointLight.quadratic", scene.plight.quadratic);

    program.setUniform("pointLight.ambient", scene.plight.ambient);
    program.setUniform("pointLight.diffuse", scene.plight.diffuse);
    program.setUniform("pointLight.specular", scene.plight.specular);

    // spotlight
    /*program.setUniform("spotLight.direction", scene.slight.direction);*/
    /*program.setUniform("spotLight.position", scene.slight.position);*/

    // flashlight
    program.setUniform("spotLight.direction", camera.front);
    program.setUniform("spotLight.position", camera.position);

    program.setUniform("spotLight.cutOff", glm::cos(glm::radians(scene.slight.cutOff)));
    program.setUniform("spotLight.outerCutOff", glm::cos(glm::radians(scene.slight.outerCutOff)));

    program.setUniform("spotLight.constant", scene.slight.constant);
    program.setUniform("spotLight.linear", scene.slight.linear);
    program.setUniform("spotLight.quadratic", scene.slight.quadratic);

    program.setUniform("spotLight.ambient", scene.slight.ambient);
    program.setUniform("spotLight.diffuse", scene.slight.diffuse);
    program.setUniform("spotLight.specular", scene.slight.specular);
}

/**
 * @brief Loads an image from the given path into an OpenGL texture.
 */
Texture loadTexture(const std::filesystem::path& path, const std::string& samplerName = "material.diffuse") {
	StbImage i;
	i.loadFromFile(path.string());
	return Texture::loadImage(i, samplerName);
}


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
	spinBunny.addAnimation(
        [&] () {
        return std::make_unique<RotationAnimation>(scene.objects[0], 10.0, glm::vec3(0, 2 * M_PI, 0));
        }
    );

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
		loadTexture("models/White_marble_03/Textures_2K/white_marble_03_2k_baseColor.tga", "material.diffuse"),
		loadTexture("models/White_marble_03/Textures_2K/white_marble_03_2k_specular.tga", "material.specular"),
	};
	auto mesh = Mesh3D::square(textures);
	auto floor = Object3D(std::vector<Mesh3D>{mesh});
	floor.grow(glm::vec3(5, 5, 5));
	floor.move(glm::vec3(0, -1.5, 0));
	floor.rotate(glm::vec3(-M_PI / 2, 0, 0));

	scene.objects.push_back(std::move(floor));
	return scene;
}

Scene testSquare() {
	Scene scene{ toonLightingShader() };

	std::vector<Texture> textures = {
		loadTexture("models/Tiles/Tiles_057_basecolor.png", "material.diffuse"),
		loadTexture("models/Tiles/Tiles_057_normal.png", "material.normal"),
        loadTexture("models/Tiles/Tiles_057_ambientOcclusion.png", "material.specular"),
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
	Scene scene{ toonLightingShader() };

	auto cube = assimpLoad("models/cube.obj", true);

	Texture tilemap = loadTexture("models/White_marble_03/Textures_2K/white_marble_03_2k_baseColor.tga", "material.diffuse");
    /*cube.addTexture(tilemap);*/

	scene.objects.push_back(std::move(cube));

	Animator spinCube;
	spinCube.addAnimation(
        [&] () {
        return std::make_unique<RotationAnimation>(scene.objects[0], 10.0, glm::vec3(0, M_PI, 0));
        }
    );
	// Then spin around the x axis.
	spinCube.addAnimation(
        [&] () {
        return std::make_unique<RotationAnimation>(scene.objects[0], 10.0, glm::vec3(M_PI, 0, 0));
        }
    );

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
	boat.move(glm::vec3(0, -0.5, 0));
	boat.grow(glm::vec3(0.01, 0.01, 0.01));
	auto tiger = assimpLoad("models/tiger/scene.gltf", true);
	tiger.move(glm::vec3(0, -5, 10));
	// Move the tiger to be a child of the boat.
	boat.addChild(std::move(tiger));

    boat.setAcceleration(glm::vec3(0.f, -1.0f, 0.f));
	// Move the boat into the scene list.
	scene.objects.push_back(std::move(boat));

	std::vector<Texture> textures = {
		// loadTexture("models/White_marble_03/Textures_2K/white_marble_03_2k_baseColor.tga", "material.diffuse"),
		// loadTexture("models/White_marble_03/Textures_2K/white_marble_03_2k_specular.tga", "material.specular"),
		// loadTexture("models/White_marble_03/Textures_2K/white_marble_03_2k_normal.tga", "material.normal"),
		loadTexture("models/Tiles/Tiles_057_basecolor.png", "material.diffuse"),
		// loadTexture("models/Tiles/Tiles_057_normal.png", "material.normal"),
	};
	auto mesh = Mesh3D::square(textures);
	auto floor = Object3D(std::vector<Mesh3D>{mesh});
	floor.grow(glm::vec3(5, 5, 5));
	floor.move(glm::vec3(0, 0, 0));
	floor.rotate(glm::vec3(-M_PI / 2, 0, 0));

	scene.objects.push_back(std::move(floor));

	// We want these animations to referenced the *moved* objects, which are no longer
	// in the variables named "tiger" and "boat". "boat" is now in the "objects" list at
	// index 0, and "tiger" is the index-1 child of the boat.
	Animator animBoat;
	animBoat.addAnimation(
        [&] () {
        return std::make_unique<RotationAnimation>(scene.objects[0], 10, glm::vec3(0, 2 * M_PI, 0));
        }
    );

	Animator animTiger;
	animTiger.addAnimation(
        [&] () {
        return std::make_unique<RotationAnimation>(scene.objects[0].getChild(1), 10, glm::vec3(0, 2 * M_PI, 0));
        }
    );

	// The Animators will be destroyed when leaving this function, so we move them into
	// a list to be returned.
	scene.animators.push_back(std::move(animBoat));
	scene.animators.push_back(std::move(animTiger));

	// Transfer ownership of the objects and animators back to the main.
	return scene;
}

Scene Room() {
	Scene scene{ toonLightingShader() };

    /*scene.plight.position = {2.f, 2.f, -3.f};*/

	auto room = assimpLoad("models/cube.obj", true);

	room.grow(glm::vec3(0.5f));
	/*lady.grow(glm::vec3(0.25));*/
	/*lady.move(glm::vec3(0, -25, -50));*/

	/*auto cube = assimpLoad("models/cube.obj", true);*/
	/*cube.move(glm::vec3(0.2, -1, -10));*/

	scene.objects.push_back(std::move(room));

	/*Animator animLady;*/
	/*animLady.addAnimation(std::make_unique<RotationAnimation>(scene.objects[0], 10, glm::vec3(2 * M_PI, 2 * M_PI, 0)));*/

	/*scene.animators.push_back(std::move(animLady));*/

    return scene;
}

void printObjectTree(const Object3D& obj, int& counter, const std::string& prefix = "", bool isLast = true) {
    // Print current node with counter
    std::cout << prefix;

    if (!prefix.empty()) {
        std::cout << (isLast ? "└── " : "├── ");
    }

    std::cout << counter++ << std::endl;

    // Prepare prefix for child nodes
    std::string childPrefix = prefix + (isLast ? "    " : "│   ");
    size_t childCount = obj.numberOfChildren();
    for (size_t i = 0; i < childCount; ++i) {
        bool lastChild = (i == childCount - 1);
        printObjectTree(obj.getChild(i), counter, childPrefix, lastChild);
    }
}

// Entry point function
void printObjectHierarchy(const Object3D& root) {
    int counter = 1;
    printObjectTree(root, counter);
}

Scene Sanders() {
    Scene scene{ toonLightingShader() };

	std::vector<Texture> textures = {
		loadTexture("models/White_marble_03/Textures_2K/white_marble_03_2k_baseColor.tga", "material.diffuse"),
		loadTexture("models/White_marble_03/Textures_2K/white_marble_03_2k_specular.tga", "material.specular"),
		loadTexture("models/White_marble_03/Textures_2K/white_marble_03_2k_normal.tga", "material.normal"),
		// loadTexture("models/Tiles/Tiles_057_basecolor.png", "material.diffuse"),
		// loadTexture("models/Tiles/Tiles_057_normal.png", "material.normal"),
	};
	auto mesh = Mesh3D::square(textures);
	auto floor = Object3D(std::vector<Mesh3D>{mesh});
	auto wall1 = Object3D(std::vector<Mesh3D>{mesh});

	floor.grow(glm::vec3(100));
	floor.move(glm::vec3(0, 0, 0));
	floor.rotate(glm::vec3(-M_PI / 2, 0, 0));

	wall1.grow(glm::vec3(100));
	wall1.move(glm::vec3(100, 0, 0));
	wall1.rotate(glm::vec3(M_PI, 0, M_PI));

	scene.objects.push_back(std::move(floor));
	scene.objects.push_back(std::move(wall1));

    auto brr = assimpLoad("models/brr/scene.gltf", true);
    auto trala = assimpLoad("models/trala/scene.gltf", true);
    auto thung = assimpLoad("models/thung/scene.gltf", true);

    brr.move(glm::vec3(0, 5, 0));

    trala.move(glm::vec3(5, 7.5f, -5));
    trala.grow(glm::vec3(0.75));
    trala.toggleGravity();

    thung.move(glm::vec3(-10, 10, -10));
    thung.grow(glm::vec3(0.75));

    scene.objects.push_back(std::move(brr));
    scene.objects.push_back(std::move(trala));
    scene.objects.push_back(std::move(thung));

    // printObjectHierarchy(scene.objects[1]);
    // printObjectHierarchy(scene.objects[1]);

    auto& obj = scene.objects[2];

    // auto& first_1 = first.getChild(0);
    // auto& first_2 = first.getChild(0).getChild(0);
    // auto& first_3 = first.getChild(0).getChild(0).getChild(0);

    // auto& first_2 = first.getChild(1);
    // auto& first_3 = first.getChild(2);

	Animator animThung;

	animThung.addAnimation(
	    [&obj] () {
        return std::make_unique<TranslationAnimation>(obj, 4,
            obj.getPosition() + glm::vec3(3.f, 2.f, 3.f));
	    }
    );

	// animBrr.addAnimation(
	//     [&] () {
	//        return std::make_unique<RotationAnimation>(first_2, 10, glm::vec3(5.0f));
	//     }
	//    );


	scene.animators.push_back(std::move(animThung));

    return scene;
}

