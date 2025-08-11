#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

out vec4 FragColor;

uniform vec3 lightPos1;
uniform vec3 lightPos2;
uniform vec3 viewPos;

uniform sampler2D shadowMap;
uniform sampler2D textureSampler;

// normal-based bias to reduce acne
float biasFromNormal(vec3 n, vec3 lightDir)
{
    return max(0.005 * (1.0 - dot(normalize(n), normalize(lightDir))), 0.0005);
}


// 3x3 PCF shadow test
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // outside light frustum -> no shadow
    if (projCoords.z > 1.0) return 0.0;

    // to [0,1]
    projCoords = projCoords * 0.5 + 0.5;

    float bias = biasFromNormal(normal, lightDir);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    for (int y = -1; y <= 1; ++y)
    {
        float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
        shadow += (projCoords.z - bias > pcfDepth) ? 1.0 : 0.0;
    }
    shadow /= 9.0;

    return clamp(shadow, 0.0, 1.0);
}

void main()
{
    vec3 albedo = texture(textureSampler, TexCoord).rgb;

    vec3 n = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    vec3 result = vec3(0.0);
    for (int i = 0; i < 2; ++i)
    {
        vec3 Lpos = (i == 0) ? lightPos1 : lightPos2;
        vec3 L    = normalize(Lpos - FragPos);

        // Phong
        vec3 ambient  = 0.2 * albedo;
        float diff    = max(dot(n, L), 0.0);
        vec3 diffuse  = diff * albedo;
        vec3 R        = reflect(-L, n);
        float spec    = pow(max(dot(V, R), 0.0), 32.0);
        vec3 specular = 0.5 * spec * vec3(1.0);

        float shadow  = ShadowCalculation(FragPosLightSpace, n, L);
        result += ambient + (1.0 - shadow) * (diffuse + specular);
    }

    FragColor = vec4(result, 1.0);
}
