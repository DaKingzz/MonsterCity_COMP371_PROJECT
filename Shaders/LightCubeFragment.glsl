#version 330 core

varying vec3 fragColor;
varying vec3 fragPosition;

void main()
{
    // simple emissive color
    gl_FragColor = vec4(fragColor, 1.0);
}
