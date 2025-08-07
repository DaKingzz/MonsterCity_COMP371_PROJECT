#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 lightPos1;
uniform vec3 lightPos2;
uniform vec3 viewPos;

uniform sampler2D textureSampler;

void main()
{
    // Material properties
    vec3 ambientColor = vec3(0.2);
    vec3 diffuseColor = texture(textureSampler, TexCoord).rgb;
    vec3 specularColor = vec3(0.5);
    float shininess = 32.0;

    // Normal
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Lighting result
    vec3 result = vec3(0.0);

    for (int i = 0; i < 2; ++i) {
        vec3 lightPos = (i == 0) ? lightPos1 : lightPos2;

        // Ambient
        vec3 ambient = ambientColor * diffuseColor;

        // Diffuse
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * diffuseColor;

        // Specular
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = specularColor * spec;

        result += ambient + diffuse + specular;
    }

    FragColor = vec4(result, 1.0);
}
