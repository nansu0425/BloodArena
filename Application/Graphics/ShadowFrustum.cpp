#include "Core/PCH.h"
#include "Graphics/ShadowFrustum.h"
#include "Math/MathUtils.h"
#include "Scene/LightComponent.h"

namespace BA
{

DirectionalShadowFrustum ComputeDirectionalShadowFrustum(
    const LightComponent& light,
    const Transform&      transform)
{
    Vector3 lightDir = Vector3::Transform(kAxisForward, transform.rotation);
    lightDir.Normalize();

    const float   halfFar  = 0.5f * light.GetShadowFarZ();
    const Vector3 lightEye = kShadowSceneCenter - lightDir * halfFar;

    Vector3 up = kAxisUp;
    if (std::abs(lightDir.Dot(kAxisUp)) > kShadowLightUpFallbackThreshold)
    {
        up = kAxisRight;
    }

    DirectionalShadowFrustum frustum = {};
    frustum.lightViewMatrix       = BuildLookAt(lightEye, kShadowSceneCenter, up);
    frustum.lightProjectionMatrix = BuildOrthographic(
        light.GetShadowOrthoWidth(),
        light.GetShadowOrthoHeight(),
        light.GetShadowNearZ(),
        light.GetShadowFarZ());
    frustum.depthBias = light.GetShadowDepthBias();

    return frustum;
}

} // namespace BA
