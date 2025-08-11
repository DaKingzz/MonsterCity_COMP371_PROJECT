#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Camera {
public:
    glm::vec3 position;
    glm::vec3 lookAt;
    glm::vec3 up;
    glm::vec3 sideVector;

    float speed;
    float fastSpeed;
    float horizontalAngle;
    float verticalAngle;
    float dt;
    float currentSpeed;
    float theta;
    float phi;
    bool cameraFirstPerson = true;

    double lastMouseX, lastMouseY;

    // Camera Class (Default Settings Creation)
    // -------------------------------
    Camera() {
        position = glm::vec3(0.6f, 1.0f, 10.0f);
        lookAt = glm::vec3(0.0f, 0.0f, -1.0f);
        up = glm::vec3(0.0f, 1.0f, 0.0f);

        speed = 1.0f;
        fastSpeed = 2.0f * speed;
        horizontalAngle = 90.0f;
        verticalAngle = 0.0f;
        currentSpeed = speed;

        lastMouseX = 0;
        lastMouseY = 0;
    }

    glm::mat4 getViewMatrix() {
        return glm::lookAt(position, position + lookAt, up);
    }

    // Update camera orientation every frame
    // -------------------------------------
    void updateOrientation(double mouseX, double mouseY, float deltaTime) {
        dt = deltaTime;
        double dx = mouseX - lastMouseX;
        double dy = mouseY - lastMouseY;
        lastMouseX = mouseX;
        lastMouseY = mouseY;

        const float angularSpeed = 60.0f;
        horizontalAngle -= dx * angularSpeed * dt;
        verticalAngle -= dy * angularSpeed * dt;

        verticalAngle = glm::clamp(verticalAngle, -85.0f, 85.0f);

        theta = glm::radians(horizontalAngle);
        phi = glm::radians(verticalAngle);

        lookAt = glm::vec3(cosf(phi) * cosf(theta), sinf(phi), -cosf(phi) * sinf(theta));
        sideVector = glm::normalize(glm::cross(lookAt, glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    glm::vec3 getPosition() const {
        return position;
    }

    glm::vec3 getUp() const {
        return up;
    }

    glm::vec3 getlookAt() const {
        return lookAt;
    }

    float getTheta(){
        return theta;
    }

    float getPhi(){
        return phi;
    }

    float getHorizontalAngle(){
        return horizontalAngle;
    }

    float getVerticalAngle(){
        return verticalAngle;
    }

    float getYaw(){
        return horizontalAngle;
    }

    // Process camera inputs
    // ---------------------
    void processInput(GLFWwindow* window) {
        bool isFast = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
        currentSpeed = isFast ? fastSpeed : speed;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            position += lookAt * dt * currentSpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            position -= lookAt * dt * currentSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            position += sideVector * dt * currentSpeed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            position -= sideVector * dt * currentSpeed;
    }
};