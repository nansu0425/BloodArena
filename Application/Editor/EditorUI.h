#pragma once

namespace BA
{

class EditorUI
{
public:
    void Initialize();
    void Shutdown();

    void Render();

private:
    void RenderHierarchy();
    void RenderInspector();

private:
    uint32_t m_selectedGameObjectId = 0;
};

extern std::unique_ptr<EditorUI> g_editorUI;

} // namespace BA
