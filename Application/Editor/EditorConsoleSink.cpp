#include "Core/PCH.h"
#include "Editor/EditorConsoleSink.h"
#include "Editor/EditorUI.h"

namespace BA
{

static LogLevel ToLogLevel(spdlog::level::level_enum level)
{
    switch (level)
    {
    case spdlog::level::trace:    return LogLevel::Trace;
    case spdlog::level::debug:    return LogLevel::Debug;
    case spdlog::level::info:     return LogLevel::Info;
    case spdlog::level::warn:     return LogLevel::Warn;
    case spdlog::level::err:      return LogLevel::Error;
    case spdlog::level::critical: return LogLevel::Critical;
    default:                      return LogLevel::Info;
    }
}

void EditorConsoleSink::sink_it_(const spdlog::details::log_msg& msg)
{
    if (!g_editorUI)
    {
        return;
    }

    spdlog::memory_buf_t formatted;
    formatter_->format(msg, formatted);

    g_editorUI->AddConsoleEntry(
        std::string(formatted.data(), formatted.size()),
        ToLogLevel(msg.level)
    );
}

void EditorConsoleSink::flush_()
{
}

} // namespace BA
