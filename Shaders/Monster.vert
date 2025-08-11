#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 texCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec4 FragPosLightSpace;

uniform mat4 worldMatrix;
uniform mat4 view = mat4(1.0f);
uniform mat4 projection = mat4(1.0f);
uniform mat4 lightSpaceMatrix;

void main()
{
    vec4 worldPosition = worldMatrix * vec4(aPos, 1.0);
    FragPos = vec3(worldPosition);
    Normal = mat3(transpose(inverse(worldMatrix))) * aNormal;
    TexCoord = texCoords;
    FragPosLightSpace = lightSpaceMatrix * worldPosition;

    gl_Position = projection * view * worldPosition;
}
