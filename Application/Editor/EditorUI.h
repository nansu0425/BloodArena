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
    bool m_showDemoWindow = true;
};

extern std::unique_ptr<EditorUI> g_editorUI;

} // namespace BA
