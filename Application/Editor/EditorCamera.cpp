#include "Core/PCH.h"
#include "Editor/EditorCamera.h"
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

void EditorCamera::Initialize()
{
    BA_LOG_INFO("EditorCamera initialized.");
}

void EditorCamera::Shutdown()
{
    BA_LOG_INFO("EditorCamera shutdown.");
}

void EditorCamera::Update(float deltaSeconds)
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

    if (g_input->IsRightMouseDown())
    {
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

        if (move.LengthSquared() > 0.0f)
        {
            move.Normalize();
            m_settings.position += move * (m_settings.moveSpeed * deltaSeconds);
        }
    }
}

Matrix EditorCamera::GetViewMatrix() const
{
    Vector3 forward = Vector3::TransformNormal(
        kAxisForward, BuildCameraOrientation(m_settings.yaw, m_settings.pitch));
    return BuildLookAt(m_settings.position, m_settings.position + forward, kAxisUp);
}

Matrix EditorCamera::GetProjectionMatrix() const
{
    return BuildPerspectiveFov(m_settings.fovY, m_aspect, m_settings.nearZ, m_settings.farZ);
}

Vector3 EditorCamera::GetPosition() const
{
    return m_settings.position;
}

void EditorCamera::SetAspect(float aspect)
{
    BA_ASSERT(aspect > 0.0f);
    m_aspect = aspect;
}

float EditorCamera::GetAspect() const
{
    return m_aspect;
}

EditorCameraSettings EditorCamera::GetSettings() const
{
    return m_settings;
}

void EditorCamera::SetSettings(const EditorCameraSettings& settings)
{
    m_settings = settings;
}

void EditorCamera::ResetToDefaults()
{
    m_settings = EditorCameraSettings{};
    m_aspect = kDefaultEditorCameraAspect;
}

} // namespace BA
