#include "Core/PCH.h"
#include "Core/Time.h"

namespace BA
{

namespace
{

constexpr float kMaxDeltaSeconds = 0.1f;

} // namespace

void Time::Initialize()
{
    LARGE_INTEGER frequency = {};
    LARGE_INTEGER counter = {};
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);

    m_frequency = frequency.QuadPart;
    m_lastCounter = counter.QuadPart;
    m_deltaSeconds = 0.0f;

    BA_LOG_INFO("Time initialized.");
}

void Time::Shutdown()
{
    BA_LOG_INFO("Time shutdown.");
}

void Time::Tick()
{
    LARGE_INTEGER counter = {};
    QueryPerformanceCounter(&counter);

    int64_t elapsed = counter.QuadPart - m_lastCounter;
    m_lastCounter = counter.QuadPart;

    m_deltaSeconds = static_cast<float>(static_cast<double>(elapsed) / static_cast<double>(m_frequency));
    if (m_deltaSeconds > kMaxDeltaSeconds)
    {
        m_deltaSeconds = kMaxDeltaSeconds;
    }
}

float Time::GetDeltaSeconds() const
{
    return m_deltaSeconds;
}

std::unique_ptr<Time> g_time;

} // namespace BA
