#include "Core/PCH.h"
#include "Graphics/ShadowFrustum.h"
#include "Graphics/Model.h"
#include "Graphics/ModelLibrary.h"
#include "Graphics/SceneBounds.h"
#include "Math/MathUtils.h"
#include "Scene/Camera.h"
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
    const Transform& lightTransform,
    const Camera&    camera,
    float            aspect)
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

    const std::array<Vector3, kAabbCornerCount> sceneCorners = GetAabbCorners(worldAabb);
    Aabb sceneLaAabb = MakeEmptyAabb();
    for (const Vector3& worldCorner : sceneCorners)
    {
        const Vector3 lightSpacePos = ProjectToLightAlignedSpace(worldCorner, frustumCenter, basis);
        sceneLaAabb = ExpandAabbWithPoint(sceneLaAabb, lightSpacePos);
    }

    const FrustumCornersWorld cameraCorners = ComputeFrustumCornersWorld(
        camera.GetViewMatrix(),
        camera.GetProjectionMatrix(aspect));
    Aabb cameraLaAabb = MakeEmptyAabb();
    for (const Vector3& worldCorner : cameraCorners.corners)
    {
        const Vector3 lightSpacePos = ProjectToLightAlignedSpace(worldCorner, frustumCenter, basis);
        cameraLaAabb = ExpandAabbWithPoint(cameraLaAabb, lightSpacePos);
    }

    // X/Y is restricted to the camera-visible receiver region. Z keeps the full
    // scene range so casters outside the camera frustum still cover their
    // receivers inside it.
    const Aabb receiverLaAabb = IntersectAabb(sceneLaAabb, cameraLaAabb);
    Aabb       fitLaAabb      = receiverLaAabb.isValid ? receiverLaAabb : sceneLaAabb;
    fitLaAabb.minCorner.z = sceneLaAabb.minCorner.z;
    fitLaAabb.maxCorner.z = sceneLaAabb.maxCorner.z;

    // Recenter frustum on the receiver's light-aligned X/Y center so the ortho
    // box aligns with the receiver region. basis.right and basis.up are
    // perpendicular to basis.forward, so this shift does not change the
    // light-aligned Z coordinates of any point.
    const float   laCenterX             = 0.5f * (fitLaAabb.minCorner.x + fitLaAabb.maxCorner.x);
    const float   laCenterY             = 0.5f * (fitLaAabb.minCorner.y + fitLaAabb.maxCorner.y);
    const Vector3 adjustedFrustumCenter = frustumCenter
                                        + basis.right * laCenterX
                                        + basis.up    * laCenterY;

    const float halfDepth = std::max(std::abs(fitLaAabb.minCorner.z), std::abs(fitLaAabb.maxCorner.z))
                          + kAutoFitDepthMargin;
    const float farZ      = 2.0f * halfDepth;
    const float nearZRaw  = fitLaAabb.minCorner.z + halfDepth - kAutoFitDepthMargin;
    const float nearZ     = std::max(nearZRaw, kAutoFitMinNearZ);

    result.isValid       = true;
    result.frustumCenter = adjustedFrustumCenter;
    result.orthoWidth    = (fitLaAabb.maxCorner.x - fitLaAabb.minCorner.x) * (1.0f + kAutoFitOrthoPaddingRatio);
    result.orthoHeight   = (fitLaAabb.maxCorner.y - fitLaAabb.minCorner.y) * (1.0f + kAutoFitOrthoPaddingRatio);
    result.nearZ         = nearZ;
    result.farZ          = farZ;

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
