#pragma once

#include "Math/Float4x4.h"
#include "Math/Angle.h"

namespace BA
{

struct CameraSettings
{
    Float3 position;
    float yaw;
    float pitch;
    float fovY;
    float nearZ;
    float farZ;
    float moveSpeed;
    float mouseSensitivity;
};

class Camera
{
public:
    void Initialize();
    void Shutdown();

    void Update(float deltaSeconds);

    Float4x4 GetViewMatrix() const;
    Float4x4 GetProjectionMatrix(float aspect) const;

    CameraSettings GetSettings() const;
    void SetSettings(const CameraSettings& settings);
    void ResetToDefaults();

private:
    // Rotation sign convention matches the left-handed X/Y rotation matrices:
    //   positive m_yaw   = turn right (rotate +X toward +Z)
    //   positive m_pitch = look down  (rotate +Y toward +Z)
    Float3 m_position = {0.0f, 0.0f, -1.0f};
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;
    float m_fovY = DegToRad(60.0f);
    float m_nearZ = 0.1f;
    float m_farZ = 1000.0f;
    float m_moveSpeed = 5.0f;
    float m_mouseSensitivity = 0.003f;
};

extern std::unique_ptr<Camera> g_camera;

} // namespace BA
