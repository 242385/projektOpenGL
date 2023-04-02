#pragma once
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>

enum CameraMovement {
    FRONT,
    BACK,
    LEFT,
    RIGHT
};

// Default values
constexpr float PITCH = 0.0f;
constexpr float YAW = -90.0f;
constexpr float ZOOM = 45.0f;

class Camera
{
public:
    glm::vec3 position;
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 upGlobal;

    // Prevents the camera from jumping around when first clicking left click
    bool firstClick = true;

    // Stores the width and height of the window
    int width;
    int height;

    // Adjust the speed of the camera and it's sensitivity when looking around
    float sensitivity = 25.0f;
    bool allowMouseLook = true;
    float baseSpeed = 1.0f;
    float speed = baseSpeed;

    // euler angles
    float pitch;
    float yaw;

    float zoom;

    // construct with vectors
    Camera(const glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), int width = 0, int height = 0,
        const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        const float yaw = YAW, const float pitch = PITCH)
    {
        this->position = position;
        this->upGlobal = up;
        this->pitch = pitch;
        this->yaw = yaw;
        this->zoom = ZOOM;
        this->width = width;
        this->height = height;
        updateCameraVectors();
    }

    glm::mat4 getViewMatrix() const
    {
        return lookAt(position, position + front, up);
    }

    void Inputs(GLFWwindow* window)
    {
        // WSAD
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            position += speed * front;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            position += speed * -front;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            position += speed * -glm::normalize(glm::cross(front, up));
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            position += speed * glm::normalize(glm::cross(front, up));

        // up and down
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            position += speed * up;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            position += speed * -up;

        // Switch focus either on Escape press or clicking LMB inside the window
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            allowMouseLook = false;
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
            allowMouseLook = true;

        // Speed up
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            speed = baseSpeed * 3.0f;
        else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
            speed = baseSpeed;

        // Mouse
        //if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        if (allowMouseLook)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

            if (firstClick)
            {
                glfwSetCursorPos(window, (width / 2), (height / 2));
                firstClick = false;
            }

            double mouseX;
            double mouseY;

            glfwGetCursorPos(window, &mouseX, &mouseY);

            float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
            float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

            glm::vec3 newOrientation = glm::rotate(front, glm::radians(-rotX), glm::normalize(glm::cross(front, up)));

            if (glm::angle(newOrientation, up) >= glm::radians(5.0f) || glm::angle(newOrientation, -up) >= glm::radians(5.0f))
                front = newOrientation;

            front = glm::rotate(front, glm::radians(-rotY), up);

            glfwSetCursorPos(window, (width / 2), (height / 2));
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            firstClick = true;
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

        // also re-calculate the Right and up vector
        right = normalize(cross(front, upGlobal));
        up = normalize(cross(right, front));
    }
};
