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

} // namespace BA
