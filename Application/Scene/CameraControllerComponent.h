#pragma once

#include "Math/MathUtils.h"
#include "Scene/IComponent.h"
#include "Scene/ITickable.h"

namespace BA
{

inline constexpr float kDefaultCameraControllerMoveSpeed        = 5.0f;
inline constexpr float kDefaultCameraControllerMouseSensitivity = 0.003f;

class CameraControllerComponent : public IComponent, public ITickable
{
public:
    CameraControllerComponent() = default;

    bool IsEnabled() const override;
    void SetEnabled(bool isEnabled) override;

    void Tick(float deltaSeconds, GameObject& owner) override;

    float GetMoveSpeed() const;
    void  SetMoveSpeed(float moveSpeed);

    float GetMouseSensitivity() const;
    void  SetMouseSensitivity(float mouseSensitivity);

    float GetYaw() const;
    void  SetYaw(float yaw);

    float GetPitch() const;
    void  SetPitch(float pitch);

private:
    bool  m_isEnabled        = true;
    float m_moveSpeed        = kDefaultCameraControllerMoveSpeed;
    float m_mouseSensitivity = kDefaultCameraControllerMouseSensitivity;
    float m_yaw              = 0.0f;
    float m_pitch            = 0.0f;
};

} // namespace BA
