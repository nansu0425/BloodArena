#include "Core/PCH.h"
#include "Core/PlaySession.h"
#include "Core/Input.h"
#include "Scene/Scene.h"

namespace BA
{

void PlaySession::Initialize()
{
    BA_LOG_INFO("PlaySession initialized.");
}

void PlaySession::Shutdown()
{
    m_editSnapshot.clear();
    m_mode = PlayMode::Edit;
    BA_LOG_INFO("PlaySession shutdown.");
}

bool PlaySession::StartPlay()
{
    BA_ASSERT(m_mode == PlayMode::Edit);

    const ActiveCameraResult activeCamera = g_scene->FindActiveCamera();
    if (!activeCamera.isFound)
    {
        BA_LOG_WARN("Cannot start Play: no enabled CameraComponent found in scene.");
        return false;
    }

    m_editSnapshot = g_scene->SerializeToString();
    m_mode = PlayMode::Playing;

    g_input->SetCursorLocked(true);

    BA_LOG_INFO("Play started.");
    return true;
}

void PlaySession::Pause()
{
    BA_ASSERT(m_mode == PlayMode::Playing);

    m_mode = PlayMode::Paused;
    g_input->SetCursorLocked(false);

    BA_LOG_INFO("Play paused.");
}

void PlaySession::Resume()
{
    BA_ASSERT(m_mode == PlayMode::Paused);

    m_mode = PlayMode::Playing;
    g_input->SetCursorLocked(true);

    BA_LOG_INFO("Play resumed.");
}

void PlaySession::Stop()
{
    BA_ASSERT(m_mode == PlayMode::Playing || m_mode == PlayMode::Paused);

    g_input->SetCursorLocked(false);

    const bool isRestored = g_scene->DeserializeFromString(m_editSnapshot);
    BA_ASSERT(isRestored);

    m_editSnapshot.clear();
    m_mode = PlayMode::Edit;

    BA_LOG_INFO("Play stopped, scene restored.");
}

PlayMode PlaySession::GetMode() const
{
    return m_mode;
}

bool PlaySession::IsPlaying() const
{
    return (m_mode == PlayMode::Playing);
}

bool PlaySession::IsPaused() const
{
    return (m_mode == PlayMode::Paused);
}

std::unique_ptr<PlaySession> g_playSession;

} // namespace BA
