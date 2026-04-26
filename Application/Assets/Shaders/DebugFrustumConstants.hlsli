#ifndef DEBUG_FRUSTUM_CONSTANTS_HLSLI
#define DEBUG_FRUSTUM_CONSTANTS_HLSLI

cbuffer DebugFrustumConstants : register(b0)
{
    row_major float4x4 frustumInverseViewProj;
    row_major float4x4 cameraView;
    row_major float4x4 cameraProjection;
    float4 color;
};

#endif
