#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoords;
layout (location = 2) in vec3 aNormal;

out vec3 vertexNormal;

uniform mat4 worldMatrix;
uniform mat4 view = mat4(1.0f);
uniform mat4 projection = mat4(1.0f);

void main(){
    vertexNormal = aNormal;
    gl_Position = projection * view * worldMatrix * vec4(aPos, 1.0);
}