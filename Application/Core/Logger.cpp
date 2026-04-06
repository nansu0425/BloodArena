#include "Core/PCH.h"
#include "Core/Logger.h"

#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace BA
{

void Logger::Initialize()
{
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
    // true -> truncate the file on open instead of appending
    sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Logs/Application.log", true)); 

    m_internalLogger = std::make_shared<spdlog::logger>("GlobalLogger", sinks.begin(), sinks.end());

    // Allows global access by name via spdlog::get("GlobalLogger")
    spdlog::register_logger(m_internalLogger);

    m_internalLogger->set_pattern("[%Y-%m-%d %H:%M:%S] [%s:%# %!] [%l] %v");
    m_internalLogger->set_level(spdlog::level::trace);
    m_internalLogger->flush_on(spdlog::level::warn);
}

void Logger::Shutdown()
{
    spdlog::shutdown();
}

spdlog::logger* Logger::GetInternalLogger() const
{
    return m_internalLogger.get();
}

std::unique_ptr<Logger> g_logger;

} // namespace BA
