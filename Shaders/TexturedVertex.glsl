#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aUV;

uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec3 vertexColor;
out vec2 vertexUV;

void main()
{
    vec4 worldPos = worldMatrix * vec4(aPos, 1.0);
    FragPos = vec3(worldPos);
    Normal = mat3(transpose(inverse(worldMatrix))) * aColor; // using aColor as fake normal
    vertexColor = aColor;
    vertexUV = aUV;
    gl_Position = projectionMatrix * viewMatrix * worldPos;
}
