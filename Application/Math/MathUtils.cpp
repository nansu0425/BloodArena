#include "Core/PCH.h"
#include "Math/MathUtils.h"

#include <cmath>

namespace BA
{

namespace
{

constexpr float kGimbalLockThreshold = 0.9999f;

// D3D11 LH NDC cube vertices. Z range is [0, 1] (note: OpenGL uses [-1, +1]).
const std::array<Vector3, kFrustumCornerCount> kNdcCubeCorners = {
    Vector3(-1.0f, -1.0f, 0.0f),
    Vector3( 1.0f, -1.0f, 0.0f),
    Vector3(-1.0f,  1.0f, 0.0f),
    Vector3( 1.0f,  1.0f, 0.0f),
    Vector3(-1.0f, -1.0f, 1.0f),
    Vector3( 1.0f, -1.0f, 1.0f),
    Vector3(-1.0f,  1.0f, 1.0f),
    Vector3( 1.0f,  1.0f, 1.0f),
};

} // namespace

Matrix BuildWorld(const Transform& transform)
{
    return Matrix::CreateScale(transform.scale)
         * Matrix::CreateFromQuaternion(transform.rotation)
         * Matrix::CreateTranslation(transform.position);
}

FrustumCornersWorld ComputeFrustumCornersWorld(const Matrix& viewMatrix,
                                               const Matrix& projectionMatrix)
{
    const Matrix inverseViewProjection = (viewMatrix * projectionMatrix).Invert();

    FrustumCornersWorld result;
    for (size_t i = 0; i < kFrustumCornerCount; ++i)
    {
        const Vector3& ndc      = kNdcCubeCorners[i];
        const Vector4  clip     = Vector4(ndc.x, ndc.y, ndc.z, 1.0f);
        const Vector4  worldHom = Vector4::Transform(clip, inverseViewProjection);
        const float    invW     = 1.0f / worldHom.w;

        result.corners[i] = Vector3(
            worldHom.x * invW,
            worldHom.y * invW,
            worldHom.z * invW);
    }

    return result;
}

Vector3 QuaternionToEulerZXY(const Quaternion& q)
{
    const float x = q.x;
    const float y = q.y;
    const float z = q.z;
    const float w = q.w;

    const float sinPitch = 2.0f * (w * x - y * z);

    Vector3 eulerRadians;
    if (std::abs(sinPitch) >= kGimbalLockThreshold)
    {
        eulerRadians.x = std::copysign(kPi * 0.5f, sinPitch);
        eulerRadians.y = std::atan2(2.0f * (w * y - x * z), 1.0f - 2.0f * (y * y + z * z));
        eulerRadians.z = 0.0f;
    }
    else
    {
        eulerRadians.x = std::asin(sinPitch);
        eulerRadians.y = std::atan2(2.0f * (x * z + w * y), 1.0f - 2.0f * (x * x + y * y));
        eulerRadians.z = std::atan2(2.0f * (x * y + w * z), 1.0f - 2.0f * (x * x + z * z));
    }

    return eulerRadians;
}

} // namespace BA
