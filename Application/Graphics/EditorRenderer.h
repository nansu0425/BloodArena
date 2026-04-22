#pragma once

#include "Math/MathTypes.h"

namespace BA
{

struct GameObject;

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
    void RenderAddComponentMenu(GameObject& gameObject);
    void RenderConsole();

    char m_consoleInputBuffer[256] = {};
    char m_sceneNameBuffer[64] = {};
    bool m_isViewportFlying = false;
    FramePhase m_framePhase = FramePhase::Idle;
};

extern std::unique_ptr<EditorRenderer> g_editorRenderer;

} // namespace BA
