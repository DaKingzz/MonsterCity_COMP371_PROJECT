#include "projectile.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <cmath>

Projectile::Projectile(glm::vec3 position, glm::vec3 velocity, int shaderProgram)
    : mPosition(position), mVelocity(velocity)
{
    mWorldMatrixLocation = glGetUniformLocation(shaderProgram, "worldMatrix");
}

void Projectile::Update(float dt) {
    mPosition += mVelocity * dt;
}

void Projectile::Draw() {
    glm::vec3 dir = glm::normalize(mVelocity);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    if (std::abs(glm::dot(dir, up)) > 0.99f) {
        up = glm::vec3(0.0f, 0.0f, 1.0f);
    }
    glm::vec3 right = glm::normalize(glm::cross(up, dir));
    glm::vec3 newUp = glm::cross(dir, right);
    glm::mat4 rotationMatrix = glm::mat4(
        glm::vec4(right, 0.0f),
        glm::vec4(newUp, 0.0f),
        glm::vec4(dir, 0.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.025f, 0.025f, 3.0f));
    glm::mat4 worldMatrix = glm::translate(glm::mat4(1.0f), mPosition) * rotationMatrix * scaleMatrix;

    glUniformMatrix4fv(mWorldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}