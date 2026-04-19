cbuffer ObjectConstants : register(b0)
{
    row_major float4x4 worldMatrix;
    row_major float4x4 viewMatrix;
    row_major float4x4 projectionMatrix;
    float4 objectColor;
    float4 baseColorFactor;
};

Texture2D diffuseMap : register(t0);
SamplerState linearWrap : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    return diffuseMap.Sample(linearWrap, input.uv) * input.color * baseColorFactor;
}
