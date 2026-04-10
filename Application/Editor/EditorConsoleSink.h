#pragma once

#include <spdlog/sinks/base_sink.h>

namespace BA
{

class EditorConsoleSink : public spdlog::sinks::base_sink<spdlog::details::null_mutex>
{
protected:
    void sink_it_(const spdlog::details::log_msg& msg) override;
    void flush_() override;
};

} // namespace BA
