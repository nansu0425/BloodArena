cbuffer ObjectConstants : register(b0)
{
    float2 objectPosition;
    float2 _padding;
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
    output.position = float4(input.position.xy + objectPosition, input.position.z, 1.0);
    output.color = objectColor;
    return output;
}
