#version 330 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 MVP;
uniform vec3 lightColor;

out vec3 fragLightColor;

void main() {
    gl_Position = MVP * vec4(vertexPosition, 1.0);
    fragLightColor = lightColor;
}
