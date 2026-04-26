#include "Core/PCH.h"
#include "Scene/LightComponent.h"

namespace BA
{

bool LightComponent::IsEnabled() const
{
    return m_isEnabled;
}

void LightComponent::SetEnabled(bool isEnabled)
{
    m_isEnabled = isEnabled;
}

LightType LightComponent::GetType() const
{
    return m_type;
}

void LightComponent::SetType(LightType type)
{
    m_type = type;
}

const Vector3& LightComponent::GetColor() const
{
    return m_color;
}

void LightComponent::SetColor(const Vector3& color)
{
    m_color = color;
}

float LightComponent::GetIntensity() const
{
    return m_intensity;
}

void LightComponent::SetIntensity(float intensity)
{
    m_intensity = intensity;
}

float LightComponent::GetSpecularStrength() const
{
    return m_specularStrength;
}

void LightComponent::SetSpecularStrength(float specularStrength)
{
    m_specularStrength = specularStrength;
}

float LightComponent::GetShininess() const
{
    return m_shininess;
}

void LightComponent::SetShininess(float shininess)
{
    m_shininess = shininess;
}

bool LightComponent::ShouldCastShadow() const
{
    return m_shouldCastShadow;
}

void LightComponent::SetShouldCastShadow(bool shouldCastShadow)
{
    m_shouldCastShadow = shouldCastShadow;
}

float LightComponent::GetShadowOrthoWidth() const
{
    return m_shadowOrthoWidth;
}

void LightComponent::SetShadowOrthoWidth(float orthoWidth)
{
    m_shadowOrthoWidth = orthoWidth;
}

float LightComponent::GetShadowOrthoHeight() const
{
    return m_shadowOrthoHeight;
}

void LightComponent::SetShadowOrthoHeight(float orthoHeight)
{
    m_shadowOrthoHeight = orthoHeight;
}

float LightComponent::GetShadowNearZ() const
{
    return m_shadowNearZ;
}

void LightComponent::SetShadowNearZ(float nearZ)
{
    m_shadowNearZ = nearZ;
}

float LightComponent::GetShadowFarZ() const
{
    return m_shadowFarZ;
}

void LightComponent::SetShadowFarZ(float farZ)
{
    m_shadowFarZ = farZ;
}

float LightComponent::GetShadowDepthBias() const
{
    return m_shadowDepthBias;
}

void LightComponent::SetShadowDepthBias(float depthBias)
{
    m_shadowDepthBias = depthBias;
}

} // namespace BA
