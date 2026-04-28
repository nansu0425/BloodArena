#include "Core/PCH.h"
#include "Scene/CameraControllerComponent.h"
#include "Scene/GameObject.h"
#include "Core/Input.h"

namespace BA
{

namespace
{

constexpr float kPitchLimit = DegToRad(89.0f);

Matrix BuildCameraOrientation(float yaw, float pitch)
{
    return Matrix::CreateRotationX(pitch) * Matrix::CreateRotationY(yaw);
}

} // namespace

bool CameraControllerComponent::IsEnabled() const
{
    return m_isEnabled;
}

void CameraControllerComponent::SetEnabled(bool isEnabled)
{
    m_isEnabled = isEnabled;
}

void CameraControllerComponent::Tick(float deltaSeconds, GameObject& owner)
{
    BA_ASSERT(g_input);

    Vector2 mouseDelta = g_input->GetMouseDelta();
    m_yaw += mouseDelta.x * m_mouseSensitivity;
    m_pitch += mouseDelta.y * m_mouseSensitivity;
    m_pitch = std::clamp(m_pitch, -kPitchLimit, kPitchLimit);

    Matrix orientation = BuildCameraOrientation(m_yaw, m_pitch);
    Vector3 forward = Vector3::TransformNormal(kAxisForward, orientation);
    Vector3 right = Vector3::TransformNormal(kAxisRight, orientation);

    Vector3 move = {};
    if (g_input->IsKeyDown('W'))
    {
        move += forward;
    }
    if (g_input->IsKeyDown('S'))
    {
        move -= forward;
    }
    if (g_input->IsKeyDown('D'))
    {
        move += right;
    }
    if (g_input->IsKeyDown('A'))
    {
        move -= right;
    }
    if (g_input->IsKeyDown('E'))
    {
        move += kAxisUp;
    }
    if (g_input->IsKeyDown('Q'))
    {
        move -= kAxisUp;
    }

    Transform& transform = owner.GetTransform();
    transform.rotation = Quaternion::CreateFromYawPitchRoll(m_yaw, m_pitch, 0.0f);

    if (move.LengthSquared() > 0.0f)
    {
        move.Normalize();
        transform.position += move * (m_moveSpeed * deltaSeconds);
    }
}

float CameraControllerComponent::GetMoveSpeed() const
{
    return m_moveSpeed;
}

void CameraControllerComponent::SetMoveSpeed(float moveSpeed)
{
    m_moveSpeed = moveSpeed;
}

float CameraControllerComponent::GetMouseSensitivity() const
{
    return m_mouseSensitivity;
}

void CameraControllerComponent::SetMouseSensitivity(float mouseSensitivity)
{
    m_mouseSensitivity = mouseSensitivity;
}

float CameraControllerComponent::GetYaw() const
{
    return m_yaw;
}

void CameraControllerComponent::SetYaw(float yaw)
{
    m_yaw = yaw;
}

float CameraControllerComponent::GetPitch() const
{
    return m_pitch;
}

void CameraControllerComponent::SetPitch(float pitch)
{
    m_pitch = pitch;
}

} // namespace BA
