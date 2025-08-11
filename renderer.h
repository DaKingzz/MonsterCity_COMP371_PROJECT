#pragma once

#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>
#include <iostream>

class Renderer {
public:
    static void setProjectionMatrix(GLuint shaderProgram, const glm::mat4& projectionMatrix) {
        glUseProgram(shaderProgram);
        GLuint loc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(loc, 1, GL_FALSE, &projectionMatrix[0][0]);
    }

    static void setViewMatrix(GLuint shaderProgram, const glm::mat4& viewMatrix) {
        glUseProgram(shaderProgram);
        GLuint loc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(loc, 1, GL_FALSE, &viewMatrix[0][0]);
    }

    static void setWorldMatrix(GLuint shaderProgram, const glm::mat4& worldMatrix) {
        glUseProgram(shaderProgram);
        GLuint loc = glGetUniformLocation(shaderProgram, "worldMatrix");
        glUniformMatrix4fv(loc, 1, GL_FALSE, &worldMatrix[0][0]);
    }

    static void bindTexture(GLuint shaderProgram, GLuint textureID, const std::string& uniformName, GLenum textureUnitIndex){
        glUseProgram(shaderProgram);
        // We activate the specified texture unit (ex: GL_TEXTURE0 + 1 = GL_TEXTURE1)
        glActiveTexture(GL_TEXTURE0 + textureUnitIndex);
        glBindTexture(GL_TEXTURE_2D, textureID);
        GLuint location = glGetUniformLocation(shaderProgram, uniformName.c_str());
        glUniform1i(location, textureUnitIndex);

        // Check if the textureSampler is not found, happened me to once...better be safe
        if (location == -1) {
            std::cerr << "[RENDERER LOG] Uniform '" << uniformName << "' not found in program:" << shaderProgram 
                << "\nTextureID: " << textureUnitIndex << std::endl;
        }
    }
    // Clear the buffers
    // -----------------
    static void clear() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
};