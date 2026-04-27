#include "ShadowDebugOverlayConstants.hlsli"

Texture2D    sceneDepth        : register(t0);
SamplerState pointClampSampler : register(s0);

struct PsIn
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};

static const float kGridBlend     = 0.5;
static const float kHeatmapBlend  = 0.6;
static const float kSafeMinTpp    = 1e-6;

static const float3 kGridColorDark  = float3(0.15, 0.15, 0.15);
static const float3 kGridColorLight = float3(0.95, 0.95, 0.95);

static const float3 kHeatColorRed    = float3(1.0, 0.2, 0.2);
static const float3 kHeatColorOrange = float3(1.0, 0.6, 0.1);
static const float3 kHeatColorGreen  = float3(0.2, 0.9, 0.3);
static const float3 kHeatColorBlue   = float3(0.2, 0.4, 1.0);

float4 main(PsIn input) : SV_TARGET
{
    if (shadowDebugMode == BA_SHADOW_DEBUG_MODE_OFF)
    {
        return float4(0.0, 0.0, 0.0, 0.0);
    }

    // Reconstruct world position from scene depth.
    float  depth     = sceneDepth.Sample(pointClampSampler, input.uv).r;
    float2 ndcXY     = float2(input.uv.x * 2.0 - 1.0, 1.0 - input.uv.y * 2.0);
    float4 worldH    = mul(float4(ndcXY, depth, 1.0), inverseViewProjection);
    float3 worldPos  = worldH.xyz / worldH.w;

    // Project into light space.
    float4 lightView = mul(float4(worldPos, 1.0), lightViewMatrix);
    float4 lightClip = mul(lightView, lightProjectionMatrix);
    float3 lndc      = lightClip.xyz / lightClip.w;
    float2 shadowUv  = float2(lndc.x * 0.5 + 0.5, -lndc.y * 0.5 + 0.5);

    // Derivatives must be evaluated under quad-uniform control flow, so compute
    // them before any divergent branch. The values are unused outside the heatmap
    // path; the compiler eliminates them when the grid path is taken.
    float2 duvdx = ddx(shadowUv);
    float2 duvdy = ddy(shadowUv);

    // Outside the light frustum: paint nothing so the receiver shows through.
    if (lndc.z > 1.0 || any(abs(lndc.xy) > 1.0))
    {
        return float4(0.0, 0.0, 0.0, 0.0);
    }

    if (shadowDebugMode == BA_SHADOW_DEBUG_MODE_TEXEL_GRID)
    {
        float2 texelCoord = floor(shadowUv * shadowMapResolution);
        float  checker    = fmod(texelCoord.x + texelCoord.y, 2.0);
        float3 gridColor  = lerp(kGridColorDark, kGridColorLight, checker);
        
        return float4(gridColor, kGridBlend);
    }

    if (shadowDebugMode == BA_SHADOW_DEBUG_MODE_PPT_HEATMAP)
    {
        float tpp = max(length(duvdx), length(duvdy)) * shadowMapResolution;
        float ppt = 1.0 / max(tpp, kSafeMinTpp);

        float3 heatColor;
        if (ppt >= pptThresholdRed)
        {
            heatColor = kHeatColorRed;
        }
        else if (ppt >= pptThresholdOrange)
        {
            heatColor = kHeatColorOrange;
        }
        else if (ppt >= pptThresholdGreen)
        {
            heatColor = kHeatColorGreen;
        }
        else
        {
            heatColor = kHeatColorBlue;
        }
        
        return float4(heatColor, kHeatmapBlend);
    }

    return float4(0.0, 0.0, 0.0, 0.0);
}
