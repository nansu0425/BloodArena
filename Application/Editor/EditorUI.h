#pragma once

#include "Core/Logger.h"

namespace BA
{

struct ConsoleEntry
{
    std::string message;
    LogLevel level;
};

class EditorUI
{
public:
    void Initialize();
    void Shutdown();

    uint32_t GetSelectedGameObjectId() const;
    void SetSelectedGameObjectId(uint32_t id);

    void AddConsoleEntry(std::string message, LogLevel level);
    void ClearConsole();
    const std::vector<ConsoleEntry>& GetConsoleEntries() const;

    LogLevel GetConsoleFilterLevel() const;
    void SetConsoleFilterLevel(LogLevel level);

    bool GetConsoleAutoScroll() const;
    void SetConsoleAutoScroll(bool autoScroll);

private:
    uint32_t m_selectedGameObjectId = 0;

    std::vector<ConsoleEntry> m_consoleEntries;
    LogLevel m_consoleFilterLevel = LogLevel::Trace;
    bool m_consoleAutoScroll = true;
};

extern std::unique_ptr<EditorUI> g_editorUI;

} // namespace BA
