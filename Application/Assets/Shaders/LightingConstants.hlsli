#ifndef BA_LIGHTING_CONSTANTS_HLSLI
#define BA_LIGHTING_CONSTANTS_HLSLI

cbuffer LightingConstants : register(b2)
{
    float4 lightDirection;   // xyz = normalized world-space direction of travel; w unused
    float4 lightColor;       // rgb = directional color * intensity; a unused
    float4 ambientColor;     // rgb = ambient color * intensity; a unused
    float4 specularParams;   // x = specularStrength, y = shininess; zw unused
};

#endif // BA_LIGHTING_CONSTANTS_HLSLI
