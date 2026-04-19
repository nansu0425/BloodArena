#include "ObjectConstants.hlsli"

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

VSOutput main(VSInput input)
{
    float4 worldPos = mul(float4(input.position, 1.0), worldMatrix);
    float4 viewPos = mul(worldPos, viewMatrix);
    VSOutput output;
    output.position = mul(viewPos, projectionMatrix);
    output.color = objectColor;
    // Pass through without inverse-transpose; correct transform needed for non-uniform scale (Task 6)
    output.normal = input.normal;
    output.uv = input.uv;
    return output;
}
