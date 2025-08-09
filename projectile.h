#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <cmath>

class Projectile {
    public:
        Projectile(const glm::vec3& position, const glm::vec3& velocity, GLuint shaderProgram)
            : mWorldMatrixLocation(glGetUniformLocation(shaderProgram, "worldMatrix"))
            , mPosition(position)
            , mPrevPosition(position)
            , mVelocity(velocity) {}
    
        void Update(float dt) {
            mPrevPosition = mPosition;        // remember last frame
            mPosition += mVelocity * dt;      // integrate
        }
    
        void Draw() const {
            // Guard against zero velocity (no direction)
            float vlen2 = glm::dot(mVelocity, mVelocity);
            if (vlen2 == 0.0f) return;
    
            glm::vec3 dir = glm::normalize(mVelocity);
            glm::vec3 up  = std::abs(glm::dot(dir, glm::vec3(0,1,0))) > 0.99f
                            ? glm::vec3(0,0,1) : glm::vec3(0,1,0);
            glm::vec3 right = glm::normalize(glm::cross(up, dir));
            glm::vec3 newUp = glm::cross(dir, right);
    
            glm::mat4 rotation = glm::mat4(
                glm::vec4(right, 0.0f),   // column 0
                glm::vec4(newUp, 0.0f),   // column 1
                glm::vec4(dir,   0.0f),   // column 2
                glm::vec4(0,0,0,1)        // column 3
            );
            glm::mat4 scale  = glm::scale(glm::mat4(1.0f), glm::vec3(0.025f, 0.025f, 3.0f));
            glm::mat4 world  = glm::translate(glm::mat4(1.0f), mPosition) * rotation * scale;
    
            glUniformMatrix4fv(mWorldMatrixLocation, 1, GL_FALSE, &world[0][0]);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    
        const glm::vec3& position()     const { return mPosition; }
        const glm::vec3& prevPosition() const { return mPrevPosition; }
        const glm::vec3& velocity()     const { return mVelocity; }
    
    private:
        GLuint     mWorldMatrixLocation = 0;
        glm::vec3  mPosition{0};
        glm::vec3  mPrevPosition{0};
        glm::vec3  mVelocity{0};
    };
    

