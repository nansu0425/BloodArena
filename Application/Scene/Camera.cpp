#include "Core/PCH.h"
#include "Scene/Camera.h"
#include "Core/Input.h"
#include "Math/Axes.h"

namespace BA
{

namespace
{

constexpr float kPitchLimit = DegToRad(89.0f);

Float4x4 BuildCameraOrientation(float yaw, float pitch)
{
    return BuildRotationX(pitch) * BuildRotationY(yaw);
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
        Float2 delta = g_input->GetMouseDelta();
        m_yaw += delta.x * m_mouseSensitivity;
        m_pitch += delta.y * m_mouseSensitivity;
        m_pitch = std::clamp(m_pitch, -kPitchLimit, kPitchLimit);
    }

    Float4x4 orientation = BuildCameraOrientation(m_yaw, m_pitch);
    Float3 forward = TransformVector(kAxisForward, orientation);
    Float3 right = TransformVector(kAxisRight, orientation);

    Float3 move = {};
    if (g_input->IsKeyDown('W')) move += forward;
    if (g_input->IsKeyDown('S')) move -= forward;
    if (g_input->IsKeyDown('D')) move += right;
    if (g_input->IsKeyDown('A')) move -= right;
    if (g_input->IsKeyDown('E')) move += kAxisUp;
    if (g_input->IsKeyDown('Q')) move -= kAxisUp;

    if (LengthSquared(move) > 0.0f)
    {
        move = Normalize(move) * (m_moveSpeed * deltaSeconds);
        m_position += move;
    }
}

Float4x4 Camera::GetViewMatrix() const
{
    Float3 forward = TransformVector(kAxisForward, BuildCameraOrientation(m_yaw, m_pitch));
    return BuildLookAt(m_position, m_position + forward, kAxisUp);
}

Float4x4 Camera::GetProjectionMatrix(float aspect) const
{
    return BuildPerspective(m_fovY, aspect, m_nearZ, m_farZ);
}

CameraSettings Camera::GetSettings() const
{
    CameraSettings settings;
    settings.position = m_position;
    settings.yaw = m_yaw;
    settings.pitch = m_pitch;
    settings.fovY = m_fovY;
    settings.nearZ = m_nearZ;
    settings.farZ = m_farZ;
    settings.moveSpeed = m_moveSpeed;
    settings.mouseSensitivity = m_mouseSensitivity;
    return settings;
}

void Camera::SetSettings(const CameraSettings& settings)
{
    m_position = settings.position;
    m_yaw = settings.yaw;
    m_pitch = settings.pitch;
    m_fovY = settings.fovY;
    m_nearZ = settings.nearZ;
    m_farZ = settings.farZ;
    m_moveSpeed = settings.moveSpeed;
    m_mouseSensitivity = settings.mouseSensitivity;
}

void Camera::ResetToDefaults()
{
    *this = Camera{};
}

std::unique_ptr<Camera> g_camera;

} // namespace BA
