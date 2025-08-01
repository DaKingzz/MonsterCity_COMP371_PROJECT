#pragma once
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

const char* getVertexShaderSource();
const char* getFragmentShaderSource();
const char* getTexturedVertexShaderSource();
const char* getTexturedFragmentShaderSource();
const char* getSkyboxVertexShader();
const char* getSkyboxFragmentShader();

int compileAndLinkShaders(const char* vertexShaderSource, const char* fragmentShaderSource);
std::string readShaderFile(const char* filePath);

void setProjectionMatrix(int shaderProgram, glm::mat4 projectionMatrix);
void setViewMatrix(int shaderProgram, glm::mat4 viewMatrix);
void setWorldMatrix(int shaderProgram, glm::mat4 worldMatrix);