#include "Core/PCH.h"
#include "Editor/EditorState.h"
#include "Editor/EditorConsoleSink.h"

namespace BA
{

namespace
{

constexpr size_t kMaxConsoleEntries = 10000;
spdlog::sink_ptr s_consoleSink;

} // namespace

void EditorState::Initialize()
{
    s_consoleSink = std::make_shared<EditorConsoleSink>();
    s_consoleSink->set_pattern("[%H:%M:%S] [%l] %v");
    g_logger->AddSink(s_consoleSink);

    BA_LOG_INFO("EditorState initialized.");
}

void EditorState::Shutdown()
{
    g_logger->RemoveSink(s_consoleSink);
    s_consoleSink.reset();

    BA_LOG_INFO("EditorState shutdown.");
}

uint32_t EditorState::GetSelectedGameObjectId() const
{
    return m_selectedGameObjectId;
}

void EditorState::SetSelectedGameObjectId(uint32_t id)
{
    m_selectedGameObjectId = id;
}

void EditorState::AddConsoleEntry(std::string message, LogLevel level)
{
    if (m_consoleEntries.size() >= kMaxConsoleEntries)
    {
        m_consoleEntries.erase(m_consoleEntries.begin());
    }

    m_consoleEntries.push_back({ std::move(message), level });
}

void EditorState::ClearConsole()
{
    m_consoleEntries.clear();
}

const std::vector<ConsoleEntry>& EditorState::GetConsoleEntries() const
{
    return m_consoleEntries;
}

LogLevel EditorState::GetConsoleFilterLevel() const
{
    return m_consoleFilterLevel;
}

void EditorState::SetConsoleFilterLevel(LogLevel level)
{
    m_consoleFilterLevel = level;
}

bool EditorState::GetConsoleAutoScroll() const
{
    return m_consoleAutoScroll;
}

void EditorState::SetConsoleAutoScroll(bool autoScroll)
{
    m_consoleAutoScroll = autoScroll;
}

EditorSettings EditorState::GetEditorSettings() const
{
    EditorSettings settings;
    settings.consoleFilterLevel = m_consoleFilterLevel;
    settings.isConsoleAutoScroll = m_consoleAutoScroll;
    return settings;
}

void EditorState::SetEditorSettings(const EditorSettings& settings)
{
    m_consoleFilterLevel = settings.consoleFilterLevel;
    m_consoleAutoScroll = settings.isConsoleAutoScroll;
}

char* EditorState::GetConsoleInputBuffer()
{
    return m_consoleInputBuffer;
}

void EditorState::ClearConsoleInputBuffer()
{
    m_consoleInputBuffer[0] = '\0';
}

char* EditorState::GetSceneNameBuffer()
{
    return m_sceneNameBuffer;
}

void EditorState::ClearSceneNameBuffer()
{
    m_sceneNameBuffer[0] = '\0';
}

void EditorState::SetSceneNameBuffer(const char* name)
{
    snprintf(m_sceneNameBuffer, kEditorSceneNameBufferSize, "%s", name);
}

bool EditorState::IsViewportFlying() const
{
    return m_isViewportFlying;
}

void EditorState::SetViewportFlying(bool isFlying)
{
    m_isViewportFlying = isFlying;
}

Gizmo::Mode EditorState::GetGizmoMode() const
{
    return m_gizmoMode;
}

void EditorState::SetGizmoMode(Gizmo::Mode mode)
{
    m_gizmoMode = mode;
}

Gizmo::Space EditorState::GetGizmoSpace() const
{
    return m_gizmoSpace;
}

void EditorState::SetGizmoSpace(Gizmo::Space space)
{
    m_gizmoSpace = space;
}

std::unique_ptr<EditorState> g_editorState;

} // namespace BA
