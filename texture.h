#pragma once
#include <GL/glew.h>
#include <vector>
#include <string>

GLuint loadTexture(const char *filename);
GLuint loadCubemap(const std::vector<std::string>& faces);