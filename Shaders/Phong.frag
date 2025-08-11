#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 viewPos;
uniform vec3 lightPos1;
uniform vec3 lightPos2;
uniform vec3 overrideColor;
uniform float overrideAmount;

uniform sampler2D textureSampler;

out vec4 FragColor;

vec3 CalcLight(vec3 lightPos)
{
    vec3 ambient = vec3(0.2);
    vec3 lightColor = vec3(1.0);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * lightColor;
    

    return ambient + diffuse + specular;
}

void main()
{
    vec3 lighting = CalcLight(lightPos1) + CalcLight(lightPos2);
    vec3 textureColor = texture(textureSampler, TexCoord).rgb;

vec3 finalColor = textureColor;
  if (overrideColor != vec3(1.0)) {
       finalColor = mix(textureColor, overrideColor, 0.5);
    }
FragColor = vec4(lighting * finalColor, 1.0);
}
