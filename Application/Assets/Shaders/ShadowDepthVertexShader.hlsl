#include "ModelConstants.hlsli"
#include "ShadowConstants.hlsli"

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

float4 main(VSInput input) : SV_POSITION
{
    float4 worldPos = mul(float4(input.position, 1.0), worldMatrix);
    float4 lightView = mul(worldPos, lightViewMatrix);
    return mul(lightView, lightProjectionMatrix);
}
