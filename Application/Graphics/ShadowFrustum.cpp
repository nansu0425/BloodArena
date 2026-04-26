#include "Core/PCH.h"
#include "Graphics/ShadowFrustum.h"
#include "Graphics/Model.h"
#include "Graphics/ModelLibrary.h"
#include "Graphics/SceneBounds.h"
#include "Math/MathUtils.h"
#include "Scene/GameObject.h"
#include "Scene/LightComponent.h"
#include "Scene/ModelComponent.h"
#include "Scene/Scene.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace BA
{

namespace
{

struct LightAlignedBasis
{
    Vector3 right;
    Vector3 up;
    Vector3 forward;
};

LightAlignedBasis BuildLightAlignedBasis(const Vector3& lightDir)
{
    Vector3 up = kAxisUp;
    if (std::abs(lightDir.Dot(kAxisUp)) > kShadowLightUpFallbackThreshold)
    {
        up = kAxisRight;
    }

    Vector3 right = up.Cross(lightDir);
    right.Normalize();

    Vector3 realUp = lightDir.Cross(right);
    realUp.Normalize();

    LightAlignedBasis basis;
    basis.right   = right;
    basis.up      = realUp;
    basis.forward = lightDir;

    return basis;
}

Vector3 ProjectToLightAlignedSpace(
    const Vector3&           worldPos,
    const Vector3&           frustumCenter,
    const LightAlignedBasis& basis)
{
    const Vector3 offset = worldPos - frustumCenter;

    return Vector3(
        offset.Dot(basis.right),
        offset.Dot(basis.up),
        offset.Dot(basis.forward));
}

} // namespace

DirectionalShadowFrustum ComputeDirectionalShadowFrustum(
    const LightComponent& light,
    const Transform&      transform)
{
    Vector3 lightDir = Vector3::Transform(kAxisForward, transform.rotation);
    lightDir.Normalize();

    const LightAlignedBasis basis    = BuildLightAlignedBasis(lightDir);
    const Vector3&          center   = light.GetShadowFrustumCenter();
    const float             halfFar  = 0.5f * light.GetShadowFarZ();
    const Vector3           lightEye = center - basis.forward * halfFar;

    DirectionalShadowFrustum frustum = {};
    frustum.lightViewMatrix       = BuildLookAt(lightEye, center, basis.up);
    frustum.lightProjectionMatrix = BuildOrthographic(
        light.GetShadowOrthoWidth(),
        light.GetShadowOrthoHeight(),
        light.GetShadowNearZ(),
        light.GetShadowFarZ());
    frustum.depthBias = light.GetShadowDepthBias();

    return frustum;
}

AutoFitShadowFrustumResult ComputeAutoFitShadowFrustumParameters(
    const Scene&     scene,
    const Transform& lightTransform)
{
    BA_PROFILE_SCOPE("ComputeAutoFitShadowFrustumParameters");

    AutoFitShadowFrustumResult result = {};
    result.isValid = false;

    const Aabb worldAabb = ComputeSceneWorldAabb(scene);
    if (!worldAabb.isValid)
    {
        return result;
    }

    const Vector3 frustumCenter = 0.5f * (worldAabb.minCorner + worldAabb.maxCorner);

    Vector3 lightDir = Vector3::Transform(kAxisForward, lightTransform.rotation);
    lightDir.Normalize();
    const LightAlignedBasis basis = BuildLightAlignedBasis(lightDir);

    const std::array<Vector3, kAabbCornerCount> worldCorners = GetAabbCorners(worldAabb);

    Aabb laAabb = MakeEmptyAabb();
    for (const Vector3& worldCorner : worldCorners)
    {
        const Vector3 lightSpacePos = ProjectToLightAlignedSpace(worldCorner, frustumCenter, basis);
        laAabb = ExpandAabbWithPoint(laAabb, lightSpacePos);
    }

    const float halfDepth = std::max(std::abs(laAabb.minCorner.z), std::abs(laAabb.maxCorner.z))
                          + kAutoFitDepthMargin;
    const float farZ      = 2.0f * halfDepth;
    const float nearZRaw  = laAabb.minCorner.z + halfDepth - kAutoFitDepthMargin;
    const float nearZ     = std::max(nearZRaw, kAutoFitMinNearZ);

    result.isValid     = true;
    result.frustumCenter = frustumCenter;
    result.orthoWidth  = (laAabb.maxCorner.x - laAabb.minCorner.x) * (1.0f + kAutoFitOrthoPaddingRatio);
    result.orthoHeight = (laAabb.maxCorner.y - laAabb.minCorner.y) * (1.0f + kAutoFitOrthoPaddingRatio);
    result.nearZ       = nearZ;
    result.farZ        = farZ;

    return result;
}

bool HasAnyShadowFitMesh(const Scene& scene)
{
    for (const GameObject& obj : scene.GetGameObjects())
    {
        const ModelComponent* mc = obj.GetComponent<ModelComponent>();
        if (!mc || !mc->IsEnabled())
        {
            continue;
        }

        const Model* model = g_modelLibrary->FindModel(mc->GetModelName());
        if (model && !model->meshes.empty())
        {
            return true;
        }
    }

    return false;
}

} // namespace BA
