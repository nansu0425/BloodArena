#include "Core/PCH.h"
#include "Editor/EditorUI.h"
#include "Editor/EditorConsoleSink.h"

namespace BA
{

namespace
{

constexpr size_t kMaxConsoleEntries = 10000;
spdlog::sink_ptr s_consoleSink;

} // namespace

void EditorUI::Initialize()
{
    s_consoleSink = std::make_shared<EditorConsoleSink>();
    s_consoleSink->set_pattern("[%H:%M:%S] [%l] %v");
    g_logger->AddSink(s_consoleSink);

    BA_LOG_INFO("EditorUI initialized.");
}

void EditorUI::Shutdown()
{
    g_logger->RemoveSink(s_consoleSink);
    s_consoleSink.reset();

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

void EditorUI::AddConsoleEntry(std::string message, LogLevel level)
{
    if (m_consoleEntries.size() >= kMaxConsoleEntries)
    {
        m_consoleEntries.erase(m_consoleEntries.begin());
    }

    m_consoleEntries.push_back({ std::move(message), level });
}

void EditorUI::ClearConsole()
{
    m_consoleEntries.clear();
}

const std::vector<ConsoleEntry>& EditorUI::GetConsoleEntries() const
{
    return m_consoleEntries;
}

LogLevel EditorUI::GetConsoleFilterLevel() const
{
    return m_consoleFilterLevel;
}

void EditorUI::SetConsoleFilterLevel(LogLevel level)
{
    m_consoleFilterLevel = level;
}

bool EditorUI::GetConsoleAutoScroll() const
{
    return m_consoleAutoScroll;
}

void EditorUI::SetConsoleAutoScroll(bool autoScroll)
{
    m_consoleAutoScroll = autoScroll;
}

std::unique_ptr<EditorUI> g_editorUI;

} // namespace BA
