#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 vertexUV;

uniform vec3 lightPos;   // light source 1
uniform vec3 lightPos2;  // light source 2
uniform vec3 viewPos;
uniform sampler2D textureSampler;

out vec4 FragColor;

void main()
{
    // Texture base color
    vec3 texColor = texture(textureSampler, vertexUV).rgb;

    // Lighting components
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * texColor;

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Light 1
    vec3 lightDir1 = normalize(lightPos - FragPos);
    float diff1 = max(dot(norm, lightDir1), 0.0);
    vec3 diffuse1 = diff1 * texColor;

    vec3 reflectDir1 = reflect(-lightDir1, norm);
    float spec1 = pow(max(dot(viewDir, reflectDir1), 0.0), 32.0);
    float specularStrength = 0.5;
    vec3 specular1 = specularStrength * spec1 * vec3(1.0);

    // Light 2
    vec3 lightDir2 = normalize(lightPos2 - FragPos);
    float diff2 = max(dot(norm, lightDir2), 0.0);
    vec3 diffuse2 = diff2 * texColor;

    vec3 reflectDir2 = reflect(-lightDir2, norm);
    float spec2 = pow(max(dot(viewDir, reflectDir2), 0.0), 32.0);
    vec3 specular2 = specularStrength * spec2 * vec3(1.0);

    // Ambient + all light contributions
    vec3 result = ambient + diffuse1 + diffuse2 + specular1 + specular2;

    FragColor = vec4(result, 1.0);
}
