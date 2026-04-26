#ifndef BA_SHADOW_CONSTANTS_HLSLI
#define BA_SHADOW_CONSTANTS_HLSLI

cbuffer ShadowConstants : register(b3)
{
    row_major float4x4 lightViewMatrix;
    row_major float4x4 lightProjectionMatrix;
    float4 shadowParams;   // x = depthBias, y = isShadowEnabled (0/1), zw = unused
};

#endif // BA_SHADOW_CONSTANTS_HLSLI
