#pragma once

#include "Math/MathTypes.h"
#include "Scene/IComponent.h"

namespace BA
{

inline constexpr float kDefaultShadowOrthoSize = 30.0f;
inline constexpr float kDefaultShadowNearZ     = 1.0f;
inline constexpr float kDefaultShadowFarZ      = 100.0f;
inline constexpr float kDefaultShadowDepthBias = 0.0005f;
inline const Vector3   kDefaultShadowFrustumCenter = {0.0f, 0.0f, 0.0f};

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

    // Shadow casting parameters are Directional-only; ignored for Ambient.
    bool           ShouldCastShadow() const;
    void           SetShouldCastShadow(bool shouldCastShadow);

    float          GetShadowOrthoWidth() const;
    void           SetShadowOrthoWidth(float orthoWidth);

    float          GetShadowOrthoHeight() const;
    void           SetShadowOrthoHeight(float orthoHeight);

    float          GetShadowNearZ() const;
    void           SetShadowNearZ(float nearZ);

    float          GetShadowFarZ() const;
    void           SetShadowFarZ(float farZ);

    float          GetShadowDepthBias() const;
    void           SetShadowDepthBias(float depthBias);

    bool           IsShadowFrustumVisualized() const;
    void           SetShadowFrustumVisualized(bool isVisualized);

    const Vector3& GetShadowFrustumCenter() const;
    void           SetShadowFrustumCenter(const Vector3& frustumCenter);

    bool           IsShadowFrustumAutoFit() const;
    void           SetShadowFrustumAutoFit(bool isAutoFit);

private:
    LightType m_type             = LightType::Directional;
    Vector3   m_color            = {1.0f, 1.0f, 1.0f};
    float     m_intensity        = 1.0f;
    float     m_specularStrength = 0.5f;
    float     m_shininess        = 32.0f;
    bool      m_isEnabled        = true;

    bool      m_shouldCastShadow  = false;
    float     m_shadowOrthoWidth  = kDefaultShadowOrthoSize;
    float     m_shadowOrthoHeight = kDefaultShadowOrthoSize;
    float     m_shadowNearZ       = kDefaultShadowNearZ;
    float     m_shadowFarZ        = kDefaultShadowFarZ;
    float     m_shadowDepthBias   = kDefaultShadowDepthBias;
    Vector3   m_shadowFrustumCenter = kDefaultShadowFrustumCenter;

    bool      m_isShadowFrustumVisualized = false;
    bool      m_isShadowFrustumAutoFit    = false;
};

} // namespace BA
