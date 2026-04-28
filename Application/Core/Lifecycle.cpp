#include "Core/PCH.h"
#include "Core/Lifecycle.h"
#include "Core/Settings.h"
#include "Core/Window.h"
#include "Core/Time.h"
#include "Core/Input.h"
#include "Core/PlaySession.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/SceneRenderer.h"
#include "Graphics/ModelLibrary.h"
#include "Graphics/TextureLibrary.h"
#include "Scene/Scene.h"

#ifdef BA_EDITOR
#include "Graphics/SceneViewport.h"
#include "Editor/EditorCamera.h"
#include "Editor/EditorState.h"
#include "Graphics/EditorRenderer.h"
#include "Graphics/Gizmo/Gizmo.h"
#include "Graphics/DebugRenderer.h"
#else
#include "Scene/GameObject.h"
#include "Scene/CameraComponent.h"
#endif // BA_EDITOR

namespace BA
{

namespace
{

#ifndef BA_EDITOR
void RenderGameToBackBuffer()
{
    const ActiveCameraResult active = g_scene->FindActiveCamera();
    BA_ASSERT(active.isFound);

    active.camera->SetAspect(g_graphicsDevice->GetAspectRatio());
    const Matrix  view     = active.camera->GetViewMatrix(active.owner->GetTransform());
    const Matrix  proj     = active.camera->GetProjectionMatrix();
    const Vector3 position = active.owner->GetTransform().position;

    g_sceneRenderer->RenderShadowPass(*g_scene, view, proj);
    g_graphicsDevice->RestoreBackBuffer();
    g_sceneRenderer->RenderMainPass(*g_scene, view, proj, position);
}
#endif // !BA_EDITOR

void RenderFrame()
{
    g_graphicsDevice->BeginFrame();

#ifdef BA_EDITOR
    g_editorRenderer->BeginImGuiFrame();
    g_editorRenderer->ResolveViewportInput();
    g_editorRenderer->UpdateInputCapture();
    g_editorRenderer->RenderPanels();
    g_editorRenderer->EndImGuiFrame();
#else
    RenderGameToBackBuffer();
#endif // BA_EDITOR

    g_graphicsDevice->EndFrame();
}

void OnResize(UINT width, UINT height)
{
    g_graphicsDevice->Resize(width, height);
    RenderFrame();
}

} // namespace

void Initialize(HINSTANCE hInstance, int nShowCmd)
{
    BA_PROFILE_SCOPE("Lifecycle::Initialize");

#ifdef _DEBUG
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
    // _CrtSetBreakAlloc(allocation number); // Break at the specified allocation number during memory allocation
#endif // _DEBUG

    g_logger = std::make_unique<Logger>();
    g_logger->Initialize();

    AppSettings appSettings = LoadSettings();

    g_time = std::make_unique<Time>();
    g_time->Initialize();

    g_input = std::make_unique<Input>();
    g_input->Initialize();

    g_window = std::make_unique<Window>();
    g_window->Initialize(hInstance, nShowCmd, appSettings.window);

    g_graphicsDevice = std::make_unique<GraphicsDevice>();
    g_graphicsDevice->Initialize(g_window->GetHandle());

    g_gpuProfiler = std::make_unique<GpuProfiler>();
    g_gpuProfiler->Initialize();

    g_textureLibrary = std::make_unique<TextureLibrary>();
    g_textureLibrary->Initialize();

    g_modelLibrary = std::make_unique<ModelLibrary>();
    g_modelLibrary->Initialize();

    g_scene = std::make_unique<Scene>();
    g_scene->Initialize();

    g_sceneRenderer = std::make_unique<SceneRenderer>();
    g_sceneRenderer->Initialize();

    g_playSession = std::make_unique<PlaySession>();
    g_playSession->Initialize();

#ifdef BA_EDITOR
    g_sceneViewport = std::make_unique<SceneViewport>();
    g_sceneViewport->Initialize();
    g_sceneViewport->GetEditorCamera()->SetSettings(appSettings.editor.viewportCamera);

    g_editorState = std::make_unique<EditorState>();
    g_editorState->Initialize();
    g_editorState->SetEditorSettings(appSettings.editor);

    g_editorRenderer = std::make_unique<EditorRenderer>();
    g_editorRenderer->Initialize();

    g_debugRenderer = std::make_unique<DebugRenderer>();
    g_debugRenderer->Initialize();

    Gizmo::Initialize();
#endif // BA_EDITOR

    g_window->SetResizeCallback(OnResize);

#ifndef BA_EDITOR
    const bool isPlayStarted = g_playSession->StartPlay();
    BA_ASSERT(isPlayStarted);
#endif // !BA_EDITOR
}

int Run()
{
    MSG msg = {};
    for (;;)
    {
        g_input->BeginFrame();

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg); // Call WndProc
        }

        if (msg.message == WM_QUIT)
        {
            break;
        }

        g_time->Tick();

        if (g_playSession->IsPlaying())
        {
            g_scene->Tick(g_time->GetDeltaSeconds());
        }

#ifdef BA_EDITOR
        g_editorRenderer->BeginImGuiFrame();
        g_editorRenderer->ResolveViewportInput();
        g_editorRenderer->UpdateInputCapture();
        if (g_playSession->GetMode() == PlayMode::Edit)
        {
            g_sceneViewport->Update(g_time->GetDeltaSeconds());
        }
#endif // BA_EDITOR

        g_graphicsDevice->BeginFrame();

#ifdef BA_EDITOR
        g_editorRenderer->RenderPanels();
        g_editorRenderer->EndImGuiFrame();
#else
        RenderGameToBackBuffer();
#endif // BA_EDITOR

        g_graphicsDevice->EndFrame();

        BA_PROFILE_GPU_COLLECT();
        BA_PROFILE_FRAME_MARK();
    }

    return static_cast<int>(msg.wParam);
}

void Shutdown()
{
    BA_PROFILE_SCOPE("Lifecycle::Shutdown");

    AppSettings appSettings;
    appSettings.window = g_window->GetSettings();
#ifdef BA_EDITOR
    appSettings.editor = g_editorState->GetEditorSettings();
    appSettings.editor.viewportCamera = g_sceneViewport->GetEditorCamera()->GetSettings();
#endif // BA_EDITOR
    SaveSettings(appSettings);

#ifdef BA_EDITOR
    Gizmo::Shutdown();

    g_debugRenderer->Shutdown();
    g_debugRenderer.reset();

    g_editorRenderer->Shutdown();
    g_editorRenderer.reset();

    g_editorState->Shutdown();
    g_editorState.reset();

    g_sceneViewport->Shutdown();
    g_sceneViewport.reset();
#endif // BA_EDITOR

    g_playSession->Shutdown();
    g_playSession.reset();

    g_sceneRenderer->Shutdown();
    g_sceneRenderer.reset();

    g_scene->Shutdown();
    g_scene.reset();

    g_modelLibrary->Shutdown();
    g_modelLibrary.reset();

    g_textureLibrary->Shutdown();
    g_textureLibrary.reset();

    g_gpuProfiler->Shutdown();
    g_gpuProfiler.reset();

    g_graphicsDevice->Shutdown();
    g_graphicsDevice.reset();

    g_window->Shutdown();
    g_window.reset();

    g_input->Shutdown();
    g_input.reset();

    g_time->Shutdown();
    g_time.reset();

    g_logger->Shutdown();
    g_logger.reset();
}

} // namespace BA
