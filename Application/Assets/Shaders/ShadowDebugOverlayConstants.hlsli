#ifndef BA_SHADOW_DEBUG_OVERLAY_CONSTANTS_HLSLI
#define BA_SHADOW_DEBUG_OVERLAY_CONSTANTS_HLSLI

#define BA_SHADOW_DEBUG_MODE_OFF         0
#define BA_SHADOW_DEBUG_MODE_TEXEL_GRID  1
#define BA_SHADOW_DEBUG_MODE_PPT_HEATMAP 2

cbuffer ShadowDebugOverlayConstants : register(b0)
{
    row_major float4x4 inverseViewProjection;
    row_major float4x4 lightViewMatrix;
    row_major float4x4 lightProjectionMatrix;
    float  shadowMapResolution;
    uint   shadowDebugMode;
    float  pptThresholdRed;
    float  pptThresholdOrange;
    float  pptThresholdGreen;
    float3 _shadowDebugOverlayPad;
};

#endif // BA_SHADOW_DEBUG_OVERLAY_CONSTANTS_HLSLI
