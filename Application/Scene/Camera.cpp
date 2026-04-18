#include "Core/PCH.h"
#include "Scene/Camera.h"
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

void Camera::Initialize()
{
    BA_LOG_INFO("Camera initialized.");
}

void Camera::Shutdown()
{
    BA_LOG_INFO("Camera shutdown.");
}

void Camera::Update(float deltaSeconds)
{
    BA_ASSERT(g_input);

    if (g_input->IsRightMouseDown())
    {
        Vector2 delta = g_input->GetMouseDelta();
        m_settings.yaw += delta.x * m_settings.mouseSensitivity;
        m_settings.pitch += delta.y * m_settings.mouseSensitivity;
        m_settings.pitch = std::clamp(m_settings.pitch, -kPitchLimit, kPitchLimit);
    }

    Matrix orientation = BuildCameraOrientation(m_settings.yaw, m_settings.pitch);
    Vector3 forward = Vector3::TransformNormal(kAxisForward, orientation);
    Vector3 right = Vector3::TransformNormal(kAxisRight, orientation);

    Vector3 move = {};
    if (g_input->IsKeyDown('W')) move += forward;
    if (g_input->IsKeyDown('S')) move -= forward;
    if (g_input->IsKeyDown('D')) move += right;
    if (g_input->IsKeyDown('A')) move -= right;
    if (g_input->IsKeyDown('E')) move += kAxisUp;
    if (g_input->IsKeyDown('Q')) move -= kAxisUp;

    if (move.LengthSquared() > 0.0f)
    {
        move.Normalize();
        m_settings.position += move * (m_settings.moveSpeed * deltaSeconds);
    }
}

Matrix Camera::GetViewMatrix() const
{
    Vector3 forward = Vector3::TransformNormal(
        kAxisForward, BuildCameraOrientation(m_settings.yaw, m_settings.pitch));
    return BuildLookAt(m_settings.position, m_settings.position + forward, kAxisUp);
}

Matrix Camera::GetProjectionMatrix(float aspect) const
{
    return BuildPerspectiveFov(m_settings.fovY, aspect, m_settings.nearZ, m_settings.farZ);
}

CameraSettings Camera::GetSettings() const
{
    return m_settings;
}

void Camera::SetSettings(const CameraSettings& settings)
{
    m_settings = settings;
}

void Camera::ResetToDefaults()
{
    *this = Camera{};
}

std::unique_ptr<Camera> g_camera;

} // namespace BA
