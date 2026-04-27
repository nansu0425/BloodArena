#include "Core/PCH.h"
#include "Scene/CameraComponent.h"

namespace BA
{

bool CameraComponent::IsEnabled() const
{
    return m_isEnabled;
}

void CameraComponent::SetEnabled(bool isEnabled)
{
    m_isEnabled = isEnabled;
}

float CameraComponent::GetFovY() const
{
    return m_fovY;
}

void CameraComponent::SetFovY(float fovY)
{
    m_fovY = fovY;
}

float CameraComponent::GetNearZ() const
{
    return m_nearZ;
}

void CameraComponent::SetNearZ(float nearZ)
{
    m_nearZ = nearZ;
}

float CameraComponent::GetFarZ() const
{
    return m_farZ;
}

void CameraComponent::SetFarZ(float farZ)
{
    m_farZ = farZ;
}

float CameraComponent::GetAspect() const
{
    return m_aspect;
}

void CameraComponent::SetAspect(float aspect)
{
    m_aspect = aspect;
}

bool CameraComponent::IsViewFrustumVisualized() const
{
    return m_isViewFrustumVisualized;
}

void CameraComponent::SetViewFrustumVisualized(bool isVisualized)
{
    m_isViewFrustumVisualized = isVisualized;
}

Matrix CameraComponent::GetViewMatrix(const Transform& transform) const
{
    Vector3 forward = Vector3::Transform(kAxisForward, transform.rotation);

    return BuildLookAt(transform.position, transform.position + forward, kAxisUp);
}

Matrix CameraComponent::GetProjectionMatrix() const
{
    return BuildPerspectiveFov(m_fovY, m_aspect, m_nearZ, m_farZ);
}

} // namespace BA
