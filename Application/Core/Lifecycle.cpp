#include "Core/PCH.h"
#include "Core/Lifecycle.h"
#include "Core/Window.h"
#include "Graphics/Renderer.h"
#include "Scene/Scene.h"

namespace BA
{

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

    g_renderer = std::make_unique<Renderer>();
    g_renderer->Initialize(g_window->GetHandle());

    g_scene = std::make_unique<Scene>();
    g_scene->Initialize();
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
            g_renderer->BeginFrame();
            g_renderer->EndFrame();
        }
    }

    return static_cast<int>(msg.wParam);
}

void Shutdown()
{
    g_scene->Shutdown();
    g_renderer->Shutdown();
    g_window->Shutdown();
    g_logger->Shutdown();
}

} // namespace BA
