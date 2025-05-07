#pragma once

#include <glad/glad.h>

const float MOVESPEED   = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM        = 45.0f;
const float YAW         = -90.f;
const float PITCH       = 0.f;


inline float Lerp(float a, float t, float b) {
    return a * (1 - t) + b * t;
}

inline glm::vec3 GLVec3Lerp(glm::vec3 a, float t, glm::vec3 b) {
    return glm::vec3(
        Lerp(a.x, t, b.x),
        Lerp(a.y, t, b.y),
        Lerp(a.z, t, b.z)
    );
}


class Camera {
    public:
        glm::vec3 front;
        glm::vec3 position;
        // glm::vec3 orientation;
        glm::vec3 up;
        glm::vec3 right;

        bool isFocused;

        bool isTargetting;
        glm::vec3 target;
        glm::vec3 hover;
        float targetLerp;

        // caching mats
        glm::mat4 view;
        glm::mat4 perspective;

        float yaw;
        float pitch;

        float zoom;

        float moveSpeed;
        float mouseSensitivity;


    Camera(glm::vec3 pos = glm::vec3(0.f, 0.f, 3.f), glm::vec3 worldUp = glm::vec3(0.f, 1.f, 0.f), float y = YAW, float p = PITCH) : front(glm::vec3(0.f, 0.f, 0.f)), zoom(ZOOM), moveSpeed(MOVESPEED), mouseSensitivity(SENSITIVITY) {
        position = pos;
        up = worldUp;
        yaw = y;
        pitch = p;

        isFocused = true;
        isTargetting = false;

        UpdateVectors();
        RequestView();
        RequestPerspective();
    }

    void ProcessKeyboard(glm::vec3 direction, float dt) {
        // each part of direction xyz must be [-1, 1]
        float velocity = moveSpeed * dt;

        glm::vec3 moveDelta = glm::vec3(0);

        moveDelta += direction.x * front * velocity;
        moveDelta += direction.z * right * velocity;

        moveDelta.y = 0.f;

        moveDelta += direction.y * up * velocity;

        if (moveDelta.x || moveDelta.y || moveDelta.z) {
            if (isTargetting) {
                hover += moveDelta;
            }
            else {
                position += moveDelta;
                hover = position;
            }
            RequestView();
        }
    }

    void ProcessMouseMove(float xOff, float yOff, bool limitPitch = true) {
        if (xOff || yOff) {
            if (!isTargetting && isFocused) {
                xOff *= mouseSensitivity;
                yOff *= mouseSensitivity;

                yaw += xOff;
                pitch += yOff;

                if (limitPitch) {
                    if (pitch > 89.0f) {
                        pitch = 89.0f;
                    }
                    if (pitch < -89.0f) {
                        pitch = -89.0f;
                    }
                }
                UpdateVectors();
            }
            RequestView();
        }
    }

    void ProcessMouseScroll(float yOff) {
        if (yOff) {
            zoom -= (float)yOff;
            if (zoom < 1.0f) {
                zoom = 1.0f;
            }
            if (zoom > 45.0f) {
                zoom = 45.0f;
            }

            RequestPerspective();
        }
    }

    void SetFront(glm::vec3 f) {
        front = f;

        RequestView();
    }

    void SetTarget(glm::vec3 t) {
        isTargetting = true;
        target = t;
        zoom = ZOOM;

        RequestView();
        RequestPerspective();
    }

    void DropTarget() {
        isTargetting = false;

        RequestView();
    }

    void ToggleFocus() {
        isFocused = !isFocused;
    }

    void RequestView() {
        callView = true;
    }
    void RequestPerspective() {
        callPerspective = true;
    }

    void update(float winX, float winY, float dt) {
        if (isTargetting) {
            if (targetLerp < 1.0) {
                targetLerp += 1.0 * dt;
                if (targetLerp >= 1.0) {
                    targetLerp = 1.0;
                }
                else {
                    RequestView();
                }
            }
        }
        else {
            if (targetLerp > 0.0) {
                targetLerp -= 1.0 * dt;
                if (targetLerp <= 0.0) {
                    targetLerp = 0.0;
                }
                else {
                    RequestView();
                }
            }
        }

        // expensive operation
        if (callView || targetLerp > 0.0 || targetLerp < 1.0) {
            UpdateView();

            callView = false;
        }

        if (callPerspective) {
            UpdatePerspective(winX, winY);

            callPerspective = false;
        }
    }

    private:
    bool callView;
    bool callPerspective;

    void UpdateVectors() {
        glm::vec3 newFront;
        newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newFront.y = sin(glm::radians(pitch));
        newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        front = glm::normalize(newFront);
        right = glm::normalize(glm::cross(front, up));
    }

    // call during movement
    void UpdateView() {
        glm::vec3 actualPosition = GLVec3Lerp(position, targetLerp, hover);
        glm::vec3 actualTarget = GLVec3Lerp(position + front, targetLerp, target);

        view = glm::lookAt(actualPosition, actualTarget, up);
    }

    // call during window resize
    void UpdatePerspective(float winX, float winY) {
        perspective = glm::perspective(glm::radians(static_cast<double>(zoom)), static_cast<double>(winX) / winY, 0.1, 100.0);
    }

};
