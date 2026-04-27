#pragma once
#if defined(BA_EDITOR)

namespace BA
{

enum class ShadowDebugMode : uint32_t
{
    Off        = 0,
    TexelGrid  = 1,
    PptHeatmap = 2,
};

// Heatmap thresholds for PPT (Pixel per Texel = how many screen pixels one shadow texel covers).
// Defaults: PPT > 3 = red (undersampling), 1~3 = orange, 0.3~1 = green (ideal), < 0.3 = blue (oversampling).
inline constexpr float kDefaultPptThresholdRed    = 3.0f;
inline constexpr float kDefaultPptThresholdOrange = 1.0f;
inline constexpr float kDefaultPptThresholdGreen  = 0.3f;

struct ShadowDebugSettings
{
    ShadowDebugMode mode               = ShadowDebugMode::Off;
    float           pptThresholdRed    = kDefaultPptThresholdRed;
    float           pptThresholdOrange = kDefaultPptThresholdOrange;
    float           pptThresholdGreen  = kDefaultPptThresholdGreen;
};

} // namespace BA

#endif // BA_EDITOR
