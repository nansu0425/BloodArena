cbuffer ObjectConstants : register(b0)
{
    row_major float4x4 worldMatrix;
    row_major float4x4 viewMatrix;
    row_major float4x4 projectionMatrix;
    float4 objectColor;
};

struct VSInput
{
    float3 position : POSITION;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VSOutput main(VSInput input)
{
    float4 worldPos = mul(float4(input.position, 1.0), worldMatrix);
    float4 viewPos = mul(worldPos, viewMatrix);
    VSOutput output;
    output.position = mul(viewPos, projectionMatrix);
    output.color = objectColor;
    return output;
}
