#ifndef BA_MODEL_CONSTANTS_HLSLI
#define BA_MODEL_CONSTANTS_HLSLI

#define BA_ALPHA_MODE_OPAQUE 0
#define BA_ALPHA_MODE_MASK   1

cbuffer ModelConstants : register(b0)
{
    row_major float4x4 worldMatrix;
    row_major float4x4 worldInverseTransposeMatrix;
    float4 baseColorFactor;
    float4 materialParams;  // x = alphaCutoff, y = alphaMode, zw reserved
};

#endif // BA_MODEL_CONSTANTS_HLSLI
