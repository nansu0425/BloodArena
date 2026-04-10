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
    void RenderHierarchy();
    void RenderInspector();
};

extern std::unique_ptr<EditorRenderer> g_editorRenderer;

} // namespace BA
