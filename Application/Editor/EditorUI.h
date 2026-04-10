#pragma once

namespace BA
{

class EditorUI
{
public:
    void Initialize();
    void Shutdown();

    uint32_t GetSelectedGameObjectId() const;
    void SetSelectedGameObjectId(uint32_t id);

private:
    uint32_t m_selectedGameObjectId = 0;
};

extern std::unique_ptr<EditorUI> g_editorUI;

} // namespace BA
