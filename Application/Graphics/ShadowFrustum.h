#pragma once

#include "Math/MathTypes.h"

namespace BA
{

class LightComponent;
struct Transform;

inline const Vector3 kShadowSceneCenter = {0.0f, 0.0f, 0.0f};
inline constexpr float kShadowLightUpFallbackThreshold = 0.99f;

struct DirectionalShadowFrustum
{
    Matrix lightViewMatrix;
    Matrix lightProjectionMatrix;
    float  depthBias;
};

DirectionalShadowFrustum ComputeDirectionalShadowFrustum(
    const LightComponent& light,
    const Transform&      transform);

} // namespace BA
