#pragma once

#ifndef BA_EDITOR
#error "EditorCamera.h must only be included in BA_EDITOR builds"
#endif

#include "Math/MathUtils.h"

namespace BA
{

inline constexpr float kDefaultEditorCameraAspect = 16.0f / 9.0f;

struct EditorCameraSettings
{
    Vector3 position         = {0.0f, 0.0f, -1.0f};
    float   yaw              = 0.0f;
    float   pitch            = 0.0f;
    float   fovY             = DegToRad(60.0f);
    float   nearZ            = 0.1f;
    float   farZ             = 1000.0f;
    float   moveSpeed        = 5.0f;
    float   mouseSensitivity = 0.003f;
};

class EditorCamera
{
public:
    void Initialize();
    void Shutdown();

    void Update(float deltaSeconds);

    Matrix  GetViewMatrix() const;
    Matrix  GetProjectionMatrix() const;
    Vector3 GetPosition() const;

    void  SetAspect(float aspect);
    float GetAspect() const;

    EditorCameraSettings GetSettings() const;
    void                 SetSettings(const EditorCameraSettings& settings);
    void                 ResetToDefaults();

private:
    EditorCameraSettings m_settings;
    float                m_aspect = kDefaultEditorCameraAspect;
};

} // namespace BA
