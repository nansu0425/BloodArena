#pragma once

#include "Math/MathTypes.h"

namespace BA
{

class GameObject;

class EditorRenderer
{
public:
    void Initialize();
    void Shutdown();

    void BeginImGuiFrame();
    void ResolveViewportInput();
    void UpdateInputCapture();
    void RenderPanels();
    void EndImGuiFrame();

private:
    enum class FramePhase
    {
        Idle,
        ImGuiFrameBegun,
        ViewportInputResolved,
        InputCaptureUpdated,
        PanelsRendered,
    };

    void RenderViewport();
    void RenderCameraSettingsMenu();
    void RenderHierarchy();
    void RenderInspector();
    void RenderModelComponent(GameObject& gameObject);
    void RenderLightComponent(GameObject& gameObject);
    void RenderCameraComponent(GameObject& gameObject);
    void RenderAddComponentMenu(GameObject& gameObject);
    void RenderConsole();

    FramePhase m_framePhase = FramePhase::Idle;
};

extern std::unique_ptr<EditorRenderer> g_editorRenderer;

} // namespace BA
