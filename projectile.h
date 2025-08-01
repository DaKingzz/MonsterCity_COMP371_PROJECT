#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>

class Projectile
{
public:
    Projectile(glm::vec3 position, glm::vec3 velocity, int shaderProgram);
    void Update(float dt);
    void Draw();
private:
    GLuint mWorldMatrixLocation;
    glm::vec3 mPosition;
    glm::vec3 mVelocity;
};