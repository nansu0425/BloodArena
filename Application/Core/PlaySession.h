#pragma once

namespace BA
{

enum class PlayMode
{
    Edit,
    Playing,
    Paused,
};

class PlaySession
{
public:
    void Initialize();
    void Shutdown();

    bool StartPlay();
    void Pause();
    void Resume();
    void Stop();

    PlayMode GetMode() const;
    bool IsPlaying() const;
    bool IsPaused() const;

private:
    PlayMode    m_mode = PlayMode::Edit;
    std::string m_editSnapshot;
};

extern std::unique_ptr<PlaySession> g_playSession;

} // namespace BA
