#include "Object3D.h"
#include "ShaderProgram.h"
#include <glm/ext.hpp>

glm::mat4 Object3D::buildModelMatrix() const {
	auto m = glm::translate(glm::mat4(1), m_position);
	m = glm::translate(m, m_center * m_scale);
	m = glm::rotate(m, m_orientation[2], glm::vec3(0, 0, 1));
	m = glm::rotate(m, m_orientation[0], glm::vec3(1, 0, 0));
	m = glm::rotate(m, m_orientation[1], glm::vec3(0, 1, 0));
	m = glm::scale(m, m_scale);
	m = glm::translate(m, -m_center);
	m = m * m_baseTransform;
	return m;
}

Object3D::Object3D(std::vector<Mesh3D>&& meshes)
	: Object3D(std::move(meshes), glm::mat4(1)) {
}

Object3D::Object3D(std::vector<Mesh3D>&& meshes, const glm::mat4& baseTransform)
	: m_meshes(meshes), m_position(), m_orientation(), m_scale(1.0),
	m_center(), m_forward(), m_velocity(), m_acceleration(), m_rotVelocity(), m_rotAcceleration(), m_shininess(4), m_baseTransform(baseTransform),
    m_display(true), m_gravityAffected(true)
{
}

const glm::vec3& Object3D::getPosition() const {
	return m_position;
}

glm::vec3& Object3D::getPosition() {
	return m_position;
}

const glm::vec3& Object3D::getOrientation() const {
	return m_orientation;
}

const glm::vec3& Object3D::getScale() const {
	return m_scale;
}

/**
 * @brief Gets the center of the object's rotation.
 */
const glm::vec3& Object3D::getCenter() const {
	return m_center;
}

const std::string& Object3D::getName() const {
	return m_name;
}

const glm::vec3& Object3D::getVelocity() const {
    return m_velocity;
}

const glm::vec3& Object3D::getRotVelocity() const {
    return m_rotVelocity;
}

const glm::vec3& Object3D::getAcceleration() const {
    return m_acceleration;
}

const glm::vec3& Object3D::getRotAcceleration() const {
    return m_rotAcceleration;
}

const glm::vec3& Object3D::getForward() const {
    return m_forward;
}

const float Object3D::getShininess() const {
    return m_shininess;
}

/*const glm::vec4& Object3D::getMaterial() const {*/
/*	return m_material;*/
/*}*/

size_t Object3D::numberOfChildren() const {
	return m_children.size();
}

const Object3D& Object3D::getChild(size_t index) const {
	return m_children[index];
}

Object3D& Object3D::getChild(size_t index) {
	return m_children[index];
}

void Object3D::setPosition(const glm::vec3& position) {
	m_position = position;
}

void Object3D::setOrientation(const glm::vec3& orientation) {
	m_orientation = orientation;
}

void Object3D::setScale(const glm::vec3& scale) {
	m_scale = scale;
}

/**
 * @brief Sets the center point of the object's rotation, which is otherwise a rotation around
   the origin in local space..
 */
void Object3D::setCenter(const glm::vec3& center)
{
	m_center = center;
}

void Object3D::setName(const std::string& name) {
	m_name = name;
}

void Object3D::setVelocity(const glm::vec3& vec) {
    m_velocity = vec;
}

void Object3D::setRotVelocity(const glm::vec3& vec) {
    m_rotVelocity = vec;
}

void Object3D::setAcceleration(const glm::vec3& accel) {
    m_acceleration = accel;
}

void Object3D::setRotAcceleration(const glm::vec3& accel) {
    m_rotAcceleration = accel;
}

void Object3D::setForward(const glm::vec3& vec) {
    m_forward = vec;
}

/*void Object3D::setMaterial(const glm::vec4& material) {*/
/*	m_material = material;*/
/*}*/

void Object3D::setShininess(const float value) {
    m_shininess = value;
}

void Object3D::move(const glm::vec3& offset) {
	m_position = m_position + offset;
}

void Object3D::rotate(const glm::vec3& rotation) {
	m_orientation = m_orientation + rotation;
}

void Object3D::grow(const glm::vec3& growth) {
	m_scale = m_scale * growth;
}

void Object3D::addChild(Object3D&& child) {
	m_children.emplace_back(child);
}

inline float signOf(float x) {
    return (x > 0 ? 1 : (x < 0 ? -1 : 0));
}

const bool Object3D::getDisplay() const {
    return m_display;
}

void Object3D::setDisplay(const bool v) {
    m_display = v;
}

void Object3D::toggleGravity() {
    m_gravityAffected = not m_gravityAffected;
}

void Object3D::updateForward() {
    float_t yaw = m_orientation.x;

    m_forward.x = cos(yaw);
    m_forward.y = 0.0f;
    m_forward.z = sin(yaw);

    m_forward = glm::normalize(m_forward);
}

void Object3D::tick(float_t dt) {
    const float_t friction = 1.25f;
    const float_t weight = 4.0f;
    const float_t gravity = 9.81f;
    const float_t deceleration = 2.f; // natural deceleration of movement.
    const float_t rubber = 0.5f; // how much velocity is retained during collision
    // updateForward();

    // m_position.x += m_forward.x * m_velocity.x * dt;
    // m_position.z += m_forward.z * m_velocity.z * dt;
    // m_position.y += m_velocity.y * dt;

    m_position += m_velocity * dt;
    m_velocity += m_acceleration * dt;

    m_orientation += m_rotVelocity * dt;
    m_rotVelocity += m_rotAcceleration * dt;

    // gravity when not accelerating upwards
    if (m_gravityAffected) {
        if (m_position.y > 0.0 and m_acceleration.y <= 0.0) {
            m_velocity.y += -(weight + gravity) * dt;
        }
    }

    // collision with ground.
    if (m_position.y < 0.0) {
        m_position.y = 0.0;
        m_velocity.y = -(m_velocity.y * rubber);
    }

    // decelerate velocity over time when not accelerating.
    if (not m_acceleration.x and m_velocity.x) {
        m_velocity.x -= deceleration * friction * dt * signOf(m_velocity.x);
    }
    if (not m_acceleration.y and m_velocity.y) {
        m_velocity.y -= deceleration * friction * dt * signOf(m_velocity.y);
    }
    if (not m_acceleration.z and m_velocity.z) {
        m_velocity.z -= deceleration * friction * dt * signOf(m_velocity.z);
    }

    for (auto& c : m_children) {
        c.tick(dt);
    }
}

void Object3D::render(ShaderProgram& shaderProgram) const {
    if (m_display)
        renderRecursive(shaderProgram, glm::mat4(1));
}

/**
 * @brief Renders the object and its children, recursively.
 * @param parentMatrix the model matrix of this object's parent in the model hierarchy.
 */
void Object3D::renderRecursive(ShaderProgram& shaderProgram, const glm::mat4& parentMatrix) const {
	// This object's true model matrix is the combination of its parent's matrix and the object's matrix.
	glm::mat4 trueModel = parentMatrix * buildModelMatrix();
	shaderProgram.setUniform("model", trueModel);

    shaderProgram.setUniform("material.shininess", m_shininess);
    /*shaderProgram.setUniform("material", m_material);*/

	// Render each mesh in the object.
	for (auto& mesh : m_meshes) {
		mesh.render(shaderProgram);
	}
	// Render the children of the object.
	for (auto& child : m_children) {
		child.renderRecursive(shaderProgram, trueModel);
	}
}
