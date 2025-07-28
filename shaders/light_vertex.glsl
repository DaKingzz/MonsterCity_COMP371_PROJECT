#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec2 vertexUV;

void main()
{
    FragPos = vec3(worldMatrix * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(worldMatrix))) * aNormal;
    vertexUV = aUV;

    gl_Position = projectionMatrix * viewMatrix * vec4(FragPos, 1.0);
}