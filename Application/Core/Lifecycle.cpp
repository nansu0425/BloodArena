#include "Core/PCH.h"
#include "Core/Lifecycle.h"
#include "Core/Settings.h"
#include "Core/Window.h"
#include "Core/Time.h"
#include "Core/Input.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/SceneRenderer.h"
#include "Graphics/ModelLibrary.h"
#include "Graphics/TextureLibrary.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"

#ifdef BA_EDITOR
#include "Graphics/SceneViewport.h"
#include "Editor/EditorUI.h"
#include "Graphics/EditorRenderer.h"
#endif // BA_EDITOR

namespace BA
{

namespace
{

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
    // TODO: When game modes are added, the game build will be locked to gameplay state
    g_sceneRenderer->Render(g_graphicsDevice->GetAspectRatio());
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

    g_textureLibrary = std::make_unique<TextureLibrary>();
    g_textureLibrary->Initialize();

    g_modelLibrary = std::make_unique<ModelLibrary>();
    g_modelLibrary->Initialize();

    g_camera = std::make_unique<Camera>();
    g_camera->Initialize();

    g_scene = std::make_unique<Scene>();
    g_scene->Initialize();

    g_sceneRenderer = std::make_unique<SceneRenderer>();
    g_sceneRenderer->Initialize();

#ifdef BA_EDITOR
    g_sceneViewport = std::make_unique<SceneViewport>();
    g_sceneViewport->Initialize();

    g_editorUI = std::make_unique<EditorUI>();
    g_editorUI->Initialize();
    g_editorUI->SetEditorSettings(appSettings.editor);

    g_editorRenderer = std::make_unique<EditorRenderer>();
    g_editorRenderer->Initialize();
#endif // BA_EDITOR

    g_window->SetResizeCallback(OnResize);
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

#ifdef BA_EDITOR
        g_editorRenderer->BeginImGuiFrame();
        g_editorRenderer->ResolveViewportInput();
        g_editorRenderer->UpdateInputCapture();
#endif // BA_EDITOR

        g_camera->Update(g_time->GetDeltaSeconds());

        g_graphicsDevice->BeginFrame();

#ifdef BA_EDITOR
        g_editorRenderer->RenderPanels();
        g_editorRenderer->EndImGuiFrame();
#else
        g_sceneRenderer->Render(g_graphicsDevice->GetAspectRatio());
#endif // BA_EDITOR

        g_graphicsDevice->EndFrame();
    }

    return static_cast<int>(msg.wParam);
}

void Shutdown()
{
    AppSettings appSettings;
    appSettings.window = g_window->GetSettings();
#ifdef BA_EDITOR
    appSettings.editor = g_editorUI->GetEditorSettings();
#endif // BA_EDITOR
    SaveSettings(appSettings);

#ifdef BA_EDITOR
    g_editorRenderer->Shutdown();
    g_editorRenderer.reset();

    g_editorUI->Shutdown();
    g_editorUI.reset();

    g_sceneViewport->Shutdown();
    g_sceneViewport.reset();
#endif // BA_EDITOR

    g_sceneRenderer->Shutdown();
    g_sceneRenderer.reset();

    g_scene->Shutdown();
    g_scene.reset();

    g_camera->Shutdown();
    g_camera.reset();

    g_modelLibrary->Shutdown();
    g_modelLibrary.reset();

    g_textureLibrary->Shutdown();
    g_textureLibrary.reset();

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
