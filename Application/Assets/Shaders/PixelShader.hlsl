#include "ModelConstants.hlsli"
#include "FrameConstants.hlsli"
#include "LightingConstants.hlsli"
#include "ShadowConstants.hlsli"

Texture2D diffuseMap : register(t0);
Texture2D shadowMap : register(t1);
SamplerState linearWrap : register(s0);
SamplerComparisonState shadowSampler : register(s1);

struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldNormal : NORMAL;
    float3 worldPosition : POSITION1;
    float2 uv : TEXCOORD;
};

float ComputeShadowFactor(float3 worldPos)
{
    if (shadowParams.y < 0.5)
    {
        return 1.0;
    }

    float4 lightView = mul(float4(worldPos, 1.0), lightViewMatrix);
    float4 lightClip = mul(lightView, lightProjectionMatrix);
    float3 ndc = lightClip.xyz / lightClip.w;

    if (ndc.z > 1.0)
    {
        return 1.0;
    }

    float2 shadowUv = float2(ndc.x * 0.5 + 0.5, -ndc.y * 0.5 + 0.5);
    float receiverDepth = ndc.z - shadowParams.x;
    return shadowMap.SampleCmpLevelZero(shadowSampler, shadowUv, receiverDepth);
}

float4 main(PSInput input) : SV_TARGET
{
    float4 diffuseSample = diffuseMap.Sample(linearWrap, input.uv) * baseColorFactor;

    if ((uint)materialParams.y == BA_ALPHA_MODE_MASK)
    {
        clip(diffuseSample.a - materialParams.x);
    }

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

    float shadow = ComputeShadowFactor(input.worldPosition);

    float3 ambientTerm  = diffuseSample.rgb * ambientColor.rgb;
    float3 diffuseTerm  = diffuseSample.rgb * lightColor.rgb * nDotL * shadow;
    float  specularMask = step(0.0, rawNDotL);
    float3 specularTerm = lightColor.rgb * specularParams.x * pow(nDotH, specularParams.y) * specularMask * shadow;

    float3 lit = ambientTerm + diffuseTerm + specularTerm;
    return float4(lit, diffuseSample.a);
}
