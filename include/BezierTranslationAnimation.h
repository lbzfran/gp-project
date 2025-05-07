#pragma once
#include "Animation.h"
/**
 * @brief Rotates an object at a continuous rate over an interval.
 */
class BezierTranslationAnimation : public Animation {
private:
    const glm::vec3 m_startPoint;
    const glm::vec3 m_mid1Point;
    const glm::vec3 m_mid2Point;
    const glm::vec3 m_endPoint;

	/**
	 * @brief Advance the animation by the given time interval.
	 */
	void applyAnimation(float dt) override {
        float t = currentTime() / duration();
        float invT = 1 - t;

        float tSquared = t * t;
        float invTSquared = invT * invT;

        float tCubed = tSquared * t;
        float invTCubed = invTSquared * invT;

        glm::vec3 bezierV;
        bezierV.x = (invTCubed * m_startPoint.x) +
                        (3 * invTSquared * t * m_mid1Point.x) +
                        (3 * invT * tSquared * m_mid2Point.x) +
                        (tCubed * m_endPoint.x);

        bezierV.y = (invTCubed * m_startPoint.y) +
                        (3 * invTSquared * t * m_mid1Point.y) +
                        (3 * invT * tSquared * m_mid2Point.y) +
                        (tCubed * m_endPoint.y);

        bezierV.z = (invTCubed * m_startPoint.z) +
                        (3 * invTSquared * t * m_mid1Point.z) +
                        (3 * invT * tSquared * m_mid2Point.z) +
                        (tCubed * m_endPoint.z);

        object().setPosition(bezierV);
	}

public:
	// BezierTranslationAnimation(Object3D& obj, float duration, const glm::vec3& mid1Point, const glm::vec3& mid2Point, const glm::vec3& totalMovement) :
	// 	Animation(obj, duration), m_startPoint(objectPosition), m_mid1Point(mid1Point), m_mid2Point(mid2Point), m_endPoint(glm::vec3(objectPosition + totalMovement)) {}
	BezierTranslationAnimation(Object3D& obj, float duration, const glm::vec3& startPoint, const glm::vec3& mid1Point, const glm::vec3& mid2Point, const glm::vec3& endPoint) :
		Animation(obj, duration), m_startPoint(startPoint), m_mid1Point(mid1Point), m_mid2Point(mid2Point), m_endPoint(endPoint) {}
};

