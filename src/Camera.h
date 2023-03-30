#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum CameraMovement {
    FRONT,
    BACK,
    LEFT,
    RIGHT
};

// Default values
constexpr float PITCH = 0.0f;
constexpr float YAW = -90.0f;
constexpr float SPEED = 100.0f;
constexpr float SENSITIVITY = 0.1f;
constexpr float ZOOM = 45.0f;

class Camera
{
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 upGlobal;

    // euler angles
    float pitch;
    float yaw;

    float speed;
    float sensitivity;
    float zoom;

    // construct with vectors
    Camera(const glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        const float yaw = YAW, const float pitch = PITCH)
    {
        this->position = position;
        this->upGlobal = up;
        this->pitch = pitch;
        this->yaw = yaw;
        this->front = glm::vec3(0.0f, 0.0f, -1.0f);
        this->speed = SPEED;
        this->sensitivity = SENSITIVITY;
        this->zoom = ZOOM;
        updateCameraVectors();
    }
    // construct with scalar values
    Camera(const float posX, const float posY, const float posZ, const float upX, const float upY, const float upZ, const float yaw, const float pitch)
    {
        this->position = glm::vec3(posX, posY, posZ);
        this->upGlobal = glm::vec3(upX, upY, upZ);
        this->pitch = pitch;
        this->yaw = yaw;
        this->front = glm::vec3(0.0f, 0.0f, -1.0f);
        this->speed = SPEED;
        this->sensitivity = SENSITIVITY;
        this->zoom = ZOOM;
        updateCameraVectors();
    }

    glm::mat4 getViewMatrix() const
    {
        return lookAt(position, position + front, up);
    }

    void processKeyboard(const CameraMovement direction, const float deltaTime)
    {
        const float velocity = speed * deltaTime;
        switch (direction)
        {
        case FRONT:
        {
            position += front * velocity;
            break;
        }
        case BACK:
        {
            position -= front * velocity;
            break;
        }
        case LEFT:
        {
            position -= right * velocity;
            break;
        }
        case RIGHT:
        {
            position += right * velocity;
            break;
        }
        }
    }

    void processMouse(float xOffset, float yOffset, const bool capPitch = true)
    {
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        pitch += yOffset;
        yaw += xOffset;

        if (capPitch)
        {
            if (pitch > 89.9f)
            {
                pitch = 89.9f;
            }
            if (pitch < -89.9f)
            {
                pitch = -89.9f;
            }
        }
        updateCameraVectors();
    }

    void processScroll(float yOffset)
    {
        zoom -= yOffset;
        if (zoom < 1.0f)
        {
            zoom = 1.0f;
        }
        if (zoom > 45.0f)
        {
            zoom = 45.0f;
        }
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        glm::vec3 frontVector;
        frontVector.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
        frontVector.y = sin(glm::radians(pitch));
        frontVector.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = normalize(frontVector);

        // also re-calculate the Right and Up vector
        right = normalize(cross(front, upGlobal));
        up = normalize(cross(right, front));
    }
};
