#pragma once

namespace BA
{

struct GameObject;

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
    void RenderModelPicker(GameObject& gameObject);
    void RenderConsole();

    char m_consoleInputBuffer[256] = {};
};

extern std::unique_ptr<EditorRenderer> g_editorRenderer;

} // namespace BA
