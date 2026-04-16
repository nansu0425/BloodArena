#pragma once

#include "Core/Window.h"
#include "Scene/Camera.h"

#ifdef BA_EDITOR
#include "Editor/EditorUI.h"
#endif // BA_EDITOR

namespace BA
{

struct AppSettings
{
    WindowSettings window;
    CameraSettings camera;
#ifdef BA_EDITOR
    EditorSettings editor;
#endif // BA_EDITOR
};

AppSettings LoadSettings();
void SaveSettings(const AppSettings& settings);

} // namespace BA
