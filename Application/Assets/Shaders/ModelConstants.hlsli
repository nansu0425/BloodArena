#ifndef BA_MODEL_CONSTANTS_HLSLI
#define BA_MODEL_CONSTANTS_HLSLI

cbuffer ModelConstants : register(b0)
{
    row_major float4x4 worldMatrix;
    row_major float4x4 worldInverseTransposeMatrix;
    float4 baseColorFactor;
};

#endif // BA_MODEL_CONSTANTS_HLSLI
