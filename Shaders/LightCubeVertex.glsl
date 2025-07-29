#version 330 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec3 lightColor;

out vec3 fragLightColor;

void main() {
    fragLightColor = lightColor;
    mat4 mvp = projectionMatrix * viewMatrix * worldMatrix;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
