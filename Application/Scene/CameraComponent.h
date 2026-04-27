#pragma once

#include "Math/MathTypes.h"
#include "Math/MathUtils.h"
#include "Scene/IComponent.h"

namespace BA
{

inline constexpr float kDefaultCameraFovY   = DegToRad(60.0f);
inline constexpr float kDefaultCameraNearZ  = 0.1f;
inline constexpr float kDefaultCameraFarZ   = 1000.0f;
inline constexpr float kDefaultCameraAspect = 16.0f / 9.0f;

class CameraComponent : public IComponent
{
public:
    CameraComponent() = default;

    bool IsEnabled() const override;
    void SetEnabled(bool isEnabled) override;

    float GetFovY() const;
    void  SetFovY(float fovY);

    float GetNearZ() const;
    void  SetNearZ(float nearZ);

    float GetFarZ() const;
    void  SetFarZ(float farZ);

    float GetAspect() const;
    void  SetAspect(float aspect);

    bool  IsViewFrustumVisualized() const;
    void  SetViewFrustumVisualized(bool isVisualized);

    Matrix GetViewMatrix(const Transform& transform) const;
    Matrix GetProjectionMatrix() const;

private:
    float m_fovY   = kDefaultCameraFovY;
    float m_nearZ  = kDefaultCameraNearZ;
    float m_farZ   = kDefaultCameraFarZ;
    float m_aspect = kDefaultCameraAspect;

    bool  m_isEnabled               = true;
    bool  m_isViewFrustumVisualized = false;
};

} // namespace BA
