#include "Core/PCH.h"
#include "Editor/EditorUI.h"

namespace BA
{

void EditorUI::Initialize()
{
    BA_LOG_INFO("EditorUI initialized.");
}

void EditorUI::Shutdown()
{
    BA_LOG_INFO("EditorUI shutdown.");
}

uint32_t EditorUI::GetSelectedGameObjectId() const
{
    return m_selectedGameObjectId;
}

void EditorUI::SetSelectedGameObjectId(uint32_t id)
{
    m_selectedGameObjectId = id;
}

std::unique_ptr<EditorUI> g_editorUI;

} // namespace BA
