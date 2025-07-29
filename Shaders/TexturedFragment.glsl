#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec3 vertexColor;
in vec2 vertexUV;

uniform vec3 lightPos1;
uniform vec3 lightColor1;
uniform vec3 lightPos2;
uniform vec3 lightColor2;
uniform vec3 viewPos;

uniform sampler2D textureSampler;

out vec4 FragColor;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0);

    vec3 ambientStrength = vec3(0.1);
    vec3 diffuseStrength = vec3(1.0);
    vec3 specularStrength = vec3(1.0);

    float shininess = 32.0;

    vec3 lights[2] = vec3[2](lightPos1, lightPos2);
    vec3 lightColors[2] = vec3[2](lightColor1, lightColor2);

    for(int i = 0; i < 2; ++i) {
        vec3 lightDir = normalize(lights[i] - FragPos);

        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diffuseStrength * diff * lightColors[i];

        // Specular
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = specularStrength * spec * lightColors[i];

        // Combine
        result += ambientStrength * lightColors[i] + diffuse + specular;
    }

    vec4 textureColor = texture(textureSampler, vertexUV);
    FragColor = vec4(result, 1.0) * textureColor;
}
