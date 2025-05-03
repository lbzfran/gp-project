#pragma once
#include "Animation.h"
/**
 * @brief Rotates an object at a continuous rate over an interval.
 */
class PauseAnimation : public Animation {
private:
	/**
	 * @brief How much to increment the position by each second.
	 */
	// glm::vec3 m_perSecond;
	// glm::vec3& m_objectPosition;

	/**
	 * @brief Advance the animation by the given time interval.
	 */
	void applyAnimation(float dt) override {
		// m_objectPosition += m_perSecond * dt;
        (void) dt;
	}

public:
	/**
	 * @brief Constructs a animation of a constant rotation by the given total rotation
	 * angle, linearly interpolated across the given duration.
	 */
	PauseAnimation(Object3D& obj, float duration) :
		Animation(obj, duration) {}
};

