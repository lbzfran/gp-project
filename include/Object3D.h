#pragma once
#include <memory>
#include "ShaderProgram.h"
#include "Mesh3D.h"
class Object3D {
private:
	// The object's list of meshes and children.
	std::vector<Mesh3D> m_meshes;
	std::vector<Object3D> m_children;

	// The object's position, orientation, and scale in world space.
	glm::vec3 m_position;
	glm::vec3 m_orientation;
	glm::vec3 m_scale;
	glm::vec3 m_center;

    glm::vec3 m_forward;

    // speed
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;

    glm::vec3 m_rotVelocity;
    glm::vec3 m_rotAcceleration;

	// The object's material.
	/*glm::vec4 m_material;*/
    float m_shininess;

	// The object's base transformation matrix.
	glm::mat4 m_baseTransform;

    bool m_display;
    bool m_gravityAffected;

	// Some objects from Assimp imports have a "name" field, useful for debugging.
	std::string m_name;

	// Recomputes the local->world transformation matrix.
	glm::mat4 buildModelMatrix() const;


public:
	// No default constructor; you must have a mesh to initialize an object.
	Object3D() = delete;

	Object3D(std::vector<Mesh3D>&& meshes);
	Object3D(std::vector<Mesh3D>&& meshes, const glm::mat4& baseTransform);

	// Simple accessors.
	const glm::vec3& getPosition() const;
	glm::vec3& getPosition();
	const glm::vec3& getOrientation() const;
	const glm::vec3& getScale() const;
	const glm::vec3& getCenter() const;
	const std::string& getName() const;
    const glm::vec3& getVelocity() const;
    const glm::vec3& getRotVelocity() const;
    const glm::vec3& getAcceleration() const;
    const glm::vec3& getRotAcceleration() const;
    const glm::vec3& getForward() const;
    const float getShininess() const;
    const bool getDisplay() const;
	/*const glm::vec4& getMaterial() const;*/

	// Child management.
	size_t numberOfChildren() const;
	const Object3D& getChild(size_t index) const;
	Object3D& getChild(size_t index);


	// Simple mutators.
	void setPosition(const glm::vec3& position);
	void setOrientation(const glm::vec3& orientation);
	void setScale(const glm::vec3& scale);
	void setCenter(const glm::vec3& center);
	void setName(const std::string& name);
    void setVelocity(const glm::vec3& vec);
    void setRotVelocity(const glm::vec3& vec);
    void setAcceleration(const glm::vec3& accel);
    void setRotAcceleration(const glm::vec3& accel);
    void setForward(const glm::vec3& vec);
    void setShininess(const float value);
    void setDisplay(const bool v);
	/*void setMaterial(const glm::vec4& material);*/

	// Transformations.
	void move(const glm::vec3& offset);
	void rotate(const glm::vec3& rotation);
	void grow(const glm::vec3& growth);
	void addChild(Object3D&& child);

    void updateForward();
    void toggleGravity();

    // movement
    void tick(float_t dt);

	// Rendering.
	void render(ShaderProgram& shaderProgram) const;
	void renderRecursive(ShaderProgram& shaderProgram, const glm::mat4& parentMatrix) const;
};
