#pragma once

#include <spdlog/spdlog.h>

namespace BA
{

class Logger
{
public:
    bool Initialize();
    bool Shutdown();

    spdlog::logger* GetInternalLogger() const;

private:
    std::shared_ptr<spdlog::logger> m_internalLogger;
};

extern Logger* g_logger;

} // namespace BA


#define BA_LOG(level, ...)      BA::g_logger->GetInternalLogger()->log(spdlog::source_loc{__FILE__, __LINE__, __func__}, level, __VA_ARGS__)

#define BA_LOG_TRACE(...)       BA_LOG(spdlog::level::trace, __VA_ARGS__)
#define BA_LOG_DEBUG(...)       BA_LOG(spdlog::level::debug, __VA_ARGS__)
#define BA_LOG_INFO(...)        BA_LOG(spdlog::level::info, __VA_ARGS__)
#define BA_LOG_WARN(...)        BA_LOG(spdlog::level::warn, __VA_ARGS__)
#define BA_LOG_ERROR(...)       BA_LOG(spdlog::level::err, __VA_ARGS__)
#define BA_LOG_CRITICAL(...)    BA_LOG(spdlog::level::critical, __VA_ARGS__)
