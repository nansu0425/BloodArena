#pragma once

namespace BA
{

class Time
{
public:
    void Initialize();
    void Shutdown();

    void Tick();

    float GetDeltaSeconds() const;

private:
    int64_t m_frequency = 0;
    int64_t m_lastCounter = 0;
    float m_deltaSeconds = 0.0f;
};

extern std::unique_ptr<Time> g_time;

} // namespace BA
