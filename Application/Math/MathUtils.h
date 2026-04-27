#pragma once

#include "Math/MathTypes.h"
#include <DirectXMath.h>
#include <array>
#include <numbers>

namespace BA
{

// --- Angle utilities (constexpr) ---

inline constexpr float kPi = std::numbers::pi_v<float>;

constexpr float DegToRad(float degrees)
{
    return degrees * (kPi / 180.0f);
}

constexpr float RadToDeg(float radians)
{
    return radians * (180.0f / kPi);
}

// --- Axis constants (left-handed, Y-up) ---
// +X = right, +Y = up, +Z = forward.
// These basis vectors apply to any coordinate frame in the project
// (world, local, view, etc.) since all frames share the same handedness
// and axis role assignment.

inline const Vector3 kAxisRight   = {1.0f, 0.0f, 0.0f};
inline const Vector3 kAxisUp      = {0.0f, 1.0f, 0.0f};
inline const Vector3 kAxisForward = {0.0f, 0.0f, 1.0f};

// --- Left-handed camera/projection builders ---
// SimpleMath defaults to right-handed (XNA convention).
// These call DirectXMath LH variants directly.

inline Matrix BuildLookAt(const Vector3& eye, const Vector3& target, const Vector3& up)
{
    return DirectX::XMMatrixLookAtLH(eye, target, up);
}

inline Matrix BuildPerspectiveFov(float fovY, float aspect, float nearZ, float farZ)
{
    return DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);
}

inline Matrix BuildOrthographic(float width, float height, float nearZ, float farZ)
{
    return DirectX::XMMatrixOrthographicLH(width, height, nearZ, farZ);
}

// --- Frustum corners ---

inline constexpr size_t kFrustumCornerCount = 8;

struct FrustumCornersWorld
{
    std::array<Vector3, kFrustumCornerCount> corners;
};

FrustumCornersWorld ComputeFrustumCornersWorld(const Matrix& viewMatrix,
                                               const Matrix& projectionMatrix);

// --- Transform ---

struct Transform
{
    Vector3    position = {0.0f, 0.0f, 0.0f};
    Quaternion rotation = Quaternion::Identity;
    Vector3    scale    = {1.0f, 1.0f, 1.0f};
};

Matrix BuildWorld(const Transform& transform);

// Inverse of Quaternion::CreateFromYawPitchRoll(yaw=y, pitch=x, roll=z):
// decompose q into Euler radians (x=pitch, y=yaw, z=roll) such that
// q == CreateFromYawPitchRoll(result.y, result.x, result.z).
Vector3 QuaternionToEulerZXY(const Quaternion& q);

} // namespace BA
