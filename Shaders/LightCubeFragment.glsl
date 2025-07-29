#version 330 core

in vec3 fragLightColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(fragLightColor, 1.0); // visible color of the light cube
}
