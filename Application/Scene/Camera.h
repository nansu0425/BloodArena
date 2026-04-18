#pragma once

#include "Math/MathUtils.h"

namespace BA
{

struct CameraSettings
{
    Vector3 position = {0.0f, 0.0f, -1.0f};
    float yaw = 0.0f;
    float pitch = 0.0f;
    float fovY = DegToRad(60.0f);
    float nearZ = 0.1f;
    float farZ = 1000.0f;
    float moveSpeed = 5.0f;
    float mouseSensitivity = 0.003f;
};

class Camera
{
public:
    void Initialize();
    void Shutdown();

    void Update(float deltaSeconds);

    Matrix GetViewMatrix() const;
    Matrix GetProjectionMatrix(float aspect) const;

    CameraSettings GetSettings() const;
    void SetSettings(const CameraSettings& settings);
    void ResetToDefaults();

private:
    // Rotation sign convention matches the left-handed X/Y rotation matrices:
    //   positive yaw   = turn right (rotate +X toward +Z)
    //   positive pitch = look down  (rotate +Y toward +Z)
    CameraSettings m_settings;
};

extern std::unique_ptr<Camera> g_camera;

} // namespace BA
