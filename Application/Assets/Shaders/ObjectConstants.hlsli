#ifndef BA_OBJECT_CONSTANTS_HLSLI
#define BA_OBJECT_CONSTANTS_HLSLI

cbuffer ObjectConstants : register(b0)
{
    row_major float4x4 worldMatrix;
    row_major float4x4 viewMatrix;
    row_major float4x4 projectionMatrix;
    float4 baseColorFactor;
};

#endif // BA_OBJECT_CONSTANTS_HLSLI
