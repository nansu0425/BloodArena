#pragma once

namespace BA
{

class EditorRenderer
{
public:
    void Initialize();
    void Shutdown();

    void Render();

private:
    void RenderViewport();
    void RenderCameraSettingsMenu();
    void RenderHierarchy();
    void RenderInspector();
    void RenderConsole();

    char m_consoleInputBuffer[256] = {};
};

extern std::unique_ptr<EditorRenderer> g_editorRenderer;

} // namespace BA
