#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

uniform vec3 viewPos;
uniform vec3 lightPos1;
uniform vec3 lightPos2;
uniform vec3 overrideColor;
uniform float overrideAmount;
uniform sampler2D shadowMap;
uniform sampler2D textureSampler;

out vec4 FragColor;

// simple normal-based bias to reduce acne
float biasFromNormal(vec3 n, vec3 lightDir)
{
    float bias = max(0.005 * (1.0 - dot(normalize(n), normalize(lightDir))), 0.0005);
    return bias;
}

// 3x3 PCF for softer edges + proper bounds check
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // outside the light's frustum -> no shadow
    if (projCoords.z > 1.0) return 0.0;

    // to [0,1]
    projCoords = projCoords * 0.5 + 0.5;

    float bias = biasFromNormal(normal, lightDir);

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    for (int y = -1; y <= 1; ++y)
    {
        float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
        shadow += (projCoords.z - bias > pcfDepth) ? 1.0 : 0.0;
    }
    shadow /= 9.0;

    // clamp just in case
    return clamp(shadow, 0.0, 1.0);
}

vec3 CalcLight(vec3 lightPos)
{
    vec3 ambient  = vec3(0.2);
    vec3 lightCol = vec3(1.0);

    vec3 n = normalize(Normal);
    vec3 L = normalize(lightPos - FragPos);
    float diff = max(dot(n, L), 0.0);
    vec3 diffuse  = diff * lightCol;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * lightColor;

    return ambient + diffuse + specular;
}

void main()
{
    vec3 base = texture(textureSampler, TexCoord).rgb;
    vec3 lighting = CalcLight(lightPos1) + CalcLight(lightPos2);
    vec3 textureColor = texture(textureSampler, TexCoord).rgb;
    FragColor = vec4(lighting * textureColor, 1.0);
}
