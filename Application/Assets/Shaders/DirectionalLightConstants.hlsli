#ifndef BA_DIRECTIONAL_LIGHT_CONSTANTS_HLSLI
#define BA_DIRECTIONAL_LIGHT_CONSTANTS_HLSLI

cbuffer DirectionalLightConstants : register(b2)
{
    float4 lightDirection;   // xyz = normalized world-space direction of travel; w unused
    float4 lightColor;       // rgb = color * intensity; a unused
    float4 ambientColor;     // rgb; a unused
    float4 specularParams;   // x = specularStrength, y = shininess; zw unused
};

#endif // BA_DIRECTIONAL_LIGHT_CONSTANTS_HLSLI
