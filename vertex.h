#pragma once
#include <glm/glm.hpp>

// Vertex structure for textured and colored vertices
struct TexturedColoredVertex
{
    TexturedColoredVertex(glm::vec3 _position, glm::vec3 _color, glm::vec2 _uv)
        : position(_position), color(_color), uv(_uv) {}

    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 uv;
};

// Extern declarations for geometry arrays
extern const TexturedColoredVertex texturedCubeVertexArray[];
extern const size_t texturedCubeVertexArrayCount;
extern const float cubeVertices[];
extern const float skyboxVertices[];
extern const size_t skyboxVerticesCount;