#ifndef BA_FRAME_CONSTANTS_HLSLI
#define BA_FRAME_CONSTANTS_HLSLI

#define BA_VIEW_MODE_LIT   0
#define BA_VIEW_MODE_UNLIT 1

cbuffer FrameConstants : register(b1)
{
    row_major float4x4 viewMatrix;
    row_major float4x4 projectionMatrix;
    float4 cameraPositionWorld;  // xyz = world-space camera position; w unused
    uint   viewMode;             // BA_VIEW_MODE_LIT or BA_VIEW_MODE_UNLIT
    uint3  _viewModePad;
};

#endif // BA_FRAME_CONSTANTS_HLSLI
