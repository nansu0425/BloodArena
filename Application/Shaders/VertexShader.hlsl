cbuffer ObjectConstants : register(b0)
{
    row_major float4x4 worldMatrix;
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
    VSOutput output;
    output.position = mul(float4(input.position, 1.0), worldMatrix);
    output.color = objectColor;
    return output;
}
