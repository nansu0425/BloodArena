#include "ModelConstants.hlsli"
#include "FrameConstants.hlsli"

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 worldNormal : NORMAL;
    float3 worldPosition : POSITION1;
    float2 uv : TEXCOORD;
};

VSOutput main(VSInput input)
{
    float4 worldPos4 = mul(float4(input.position, 1.0), worldMatrix);
    VSOutput output;
    output.worldPosition = worldPos4.xyz;
    output.position = mul(mul(worldPos4, viewMatrix), projectionMatrix);
    output.worldNormal = mul(float4(input.normal, 0.0), worldInverseTransposeMatrix).xyz;
    output.uv = input.uv;
    return output;
}
