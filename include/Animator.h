#pragma once
#include <memory>
#include <vector>
#include <functional>
#include "Animation.h"

class Animator {
private:
	/**
	 * @brief How much time has elapsed since the animation started.
	 */
	float m_currentTime;
	/**
	 * @brief The time at which we transition to the next animation.
	 */
	float m_nextTransition;
	/**
	 * @brief The sequence of animations to play.
	 */
    std::vector<std::function<std::unique_ptr<Animation> (void)>> m_animations;

	/**
	 * @brief The current (active) animation.
	 */
    std::unique_ptr<Animation> m_currentAnimation;
	/**
	 * @brief The index of the current animation.
	 */
	int32_t m_currentIndex;

	/**
	 * @brief Activate the next animation.
	 */
	void nextAnimation();

public:
	/**
	 * @brief Constructs an Animator that acts on the given object.
	 */
	Animator() :
		m_currentTime(0),
		m_nextTransition(0),
		m_currentAnimation(nullptr),
		m_currentIndex(-1) {
	}

	/**
	 * @brief Add an Animation to the end of the animation sequence.
	 */
	void addAnimation(std::function<std::unique_ptr<Animation> (void)> animation) {
		m_animations.emplace_back(std::move(animation));
	}

	/**
	 * @brief Activate the Animator, causing its active animation to receive future tick() calls.
	 */
	void start();

    const float getIndex() const;

	/**
	 * @brief Advance the animation sequence by the given time interval, in seconds.
	 */
	void tick(float dt);

};
