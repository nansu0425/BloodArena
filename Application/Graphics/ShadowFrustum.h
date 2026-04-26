#pragma once

#include "Math/MathTypes.h"

namespace BA
{

class LightComponent;
class Scene;
struct Transform;

inline constexpr float kShadowLightUpFallbackThreshold = 0.99f;

inline constexpr float kAutoFitOrthoPaddingRatio = 0.05f;
inline constexpr float kAutoFitDepthMargin       = 1.0f;
inline constexpr float kAutoFitMinNearZ          = 0.1f;

struct DirectionalShadowFrustum
{
    Matrix lightViewMatrix;
    Matrix lightProjectionMatrix;
    float  depthBias;
};

DirectionalShadowFrustum ComputeDirectionalShadowFrustum(
    const LightComponent& light,
    const Transform&      transform);

struct AutoFitShadowFrustumResult
{
    bool    isValid;
    Vector3 frustumCenter;
    float   orthoWidth;
    float   orthoHeight;
    float   nearZ;
    float   farZ;
};

AutoFitShadowFrustumResult ComputeAutoFitShadowFrustumParameters(
    const Scene&     scene,
    const Transform& lightTransform);

bool HasAnyShadowFitMesh(const Scene& scene);

} // namespace BA
