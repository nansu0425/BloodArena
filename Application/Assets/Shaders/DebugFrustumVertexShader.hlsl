#include "DebugFrustumConstants.hlsli"

struct VSInput
{
    float3 position : POSITION;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};

VSOutput main(VSInput input)
{
    float4 ndc = float4(input.position, 1.0);

    float4 worldH = mul(ndc, frustumInverseViewProj);
    float3 worldPos = worldH.xyz / worldH.w;

    float4 viewPos = mul(float4(worldPos, 1.0), cameraView);
    float4 clipPos = mul(viewPos, cameraProjection);

    VSOutput output;
    output.position = clipPos;
    return output;
}
