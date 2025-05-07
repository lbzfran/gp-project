#pragma once
#include "Animation.h"
/**
 * @brief Rotates an object at a continuous rate over an interval.
 */
class TranslationAnimation : public Animation {
private:
	/**
	 * @brief How much to increment the position by each second.
	 */
	glm::vec3 m_perSecond;

	/**
	 * @brief Advance the animation by the given time interval.
	 */
	void applyAnimation(float dt) override {
		object().move(m_perSecond * dt);
	}

public:
	/**
	 * @brief Constructs a animation of a constant rotation by the given total rotation
	 * angle, linearly interpolated across the given duration.
	 */
	TranslationAnimation(Object3D& obj, float duration, const glm::vec3& totalMovement) :
		Animation(obj, duration), m_perSecond(totalMovement / duration) {}
};

