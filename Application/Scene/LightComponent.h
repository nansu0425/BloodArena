#pragma once

#include "Math/MathTypes.h"
#include "Scene/IComponent.h"

namespace BA
{

enum class LightType
{
    Directional,
    Ambient,
};

class LightComponent : public IComponent
{
public:
    LightComponent() = default;

    bool IsEnabled() const override;
    void SetEnabled(bool isEnabled) override;

    LightType      GetType() const;
    void           SetType(LightType type);

    const Vector3& GetColor() const;
    void           SetColor(const Vector3& color);

    float          GetIntensity() const;
    void           SetIntensity(float intensity);

    // specularStrength, shininess are Directional-only; ignored for Ambient.
    float          GetSpecularStrength() const;
    void           SetSpecularStrength(float specularStrength);

    float          GetShininess() const;
    void           SetShininess(float shininess);

private:
    LightType m_type             = LightType::Directional;
    Vector3   m_color            = {1.0f, 1.0f, 1.0f};
    float     m_intensity        = 1.0f;
    float     m_specularStrength = 0.5f;
    float     m_shininess        = 32.0f;
    bool      m_isEnabled        = true;
};

} // namespace BA
