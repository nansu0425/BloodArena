#include "ModelConstants.hlsli"
#include "FrameConstants.hlsli"
#include "DirectionalLightConstants.hlsli"

Texture2D diffuseMap : register(t0);
SamplerState linearWrap : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldNormal : NORMAL;
    float3 worldPosition : POSITION1;
    float2 uv : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 diffuseSample = diffuseMap.Sample(linearWrap, input.uv) * baseColorFactor;

    if (viewMode == BA_VIEW_MODE_UNLIT)
    {
        return diffuseSample;
    }

    float3 N = normalize(input.worldNormal);
    float3 L = normalize(-lightDirection.xyz);
    float3 V = normalize(cameraPositionWorld.xyz - input.worldPosition);
    float3 H = normalize(L + V);
    float  rawNDotL = dot(N, L);
    float  nDotL = saturate(rawNDotL);
    float  nDotH = saturate(dot(N, H));

    float3 ambientTerm = diffuseSample.rgb * ambientColor.rgb;
    float3 diffuseTerm = diffuseSample.rgb * lightColor.rgb * nDotL;
    float  specularMask = step(0.0, rawNDotL);
    float3 specularTerm = lightColor.rgb * specularParams.x * pow(nDotH, specularParams.y) * specularMask;

    float3 lit = ambientTerm + diffuseTerm + specularTerm;
    return float4(lit, diffuseSample.a);
}
