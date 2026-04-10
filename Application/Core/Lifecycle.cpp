#include "Core/PCH.h"
#include "Core/Lifecycle.h"
#include "Core/Window.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/SceneRenderer.h"
#include "Scene/Scene.h"
#include "Editor/EditorUI.h"
#include "Graphics/EditorRenderer.h"

namespace BA
{

static void RenderFrame()
{
    g_graphicsDevice->BeginFrame();
    g_sceneRenderer->Render();
    g_editorRenderer->Render();
    g_graphicsDevice->EndFrame();
}

static void OnResize(UINT width, UINT height)
{
    g_graphicsDevice->Resize(width, height);
    RenderFrame();
}

void Initialize(HINSTANCE hInstance, int nShowCmd)
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
    // _CrtSetBreakAlloc(allocation number); // Break at the specified allocation number during memory allocation
#endif // _DEBUG

    g_logger = std::make_unique<Logger>();
    g_logger->Initialize();

    g_window = std::make_unique<Window>();
    g_window->Initialize(hInstance, nShowCmd);

    g_graphicsDevice = std::make_unique<GraphicsDevice>();
    g_graphicsDevice->Initialize(g_window->GetHandle());

    g_scene = std::make_unique<Scene>();
    g_scene->Initialize();

    g_sceneRenderer = std::make_unique<SceneRenderer>();
    g_sceneRenderer->Initialize();

    g_editorUI = std::make_unique<EditorUI>();
    g_editorUI->Initialize();

    g_editorRenderer = std::make_unique<EditorRenderer>();
    g_editorRenderer->Initialize();

    g_window->SetResizeCallback(OnResize);
}

int Run()
{
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg); // Call WndProc
        }
        else
        {
            RenderFrame();
        }
    }

    return static_cast<int>(msg.wParam);
}

void Shutdown()
{
    g_editorRenderer->Shutdown();
    g_editorRenderer.reset();

    g_editorUI->Shutdown();
    g_editorUI.reset();

    g_sceneRenderer->Shutdown();
    g_sceneRenderer.reset();

    g_scene->Shutdown();
    g_scene.reset();

    g_graphicsDevice->Shutdown();
    g_graphicsDevice.reset();

    g_window->Shutdown();
    g_window.reset();

    g_logger->Shutdown();
    g_logger.reset();
}

} // namespace BA
