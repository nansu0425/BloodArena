#include "Core/PCH.h"
#include "Core/Window.h"
#include "Graphics/Renderer.h"

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE /*hPrevInstance*/,
    _In_ LPWSTR /*lpCmdLine*/,
    _In_ int nShowCmd
)
{
    ////////////////////////////////////////////////////
    //              Initialization Phase              //
    ////////////////////////////////////////////////////

#ifdef _DEBUG
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
    // _CrtSetBreakAlloc(allocation number); // Break at the specified allocation number during memory allocation
#endif // _DEBUG

    BA::g_logger = new BA::Logger();
    if (BA::g_logger->Initialize() == false)
    {
        BA_CRASH();
    }

    BA::g_window = std::make_unique<BA::Window>();
    BA::g_window->Initialize(hInstance, nShowCmd);

    BA::g_renderer = std::make_unique<BA::Renderer>();
    BA::g_renderer->Initialize(BA::g_window->GetHandle());

    ///////////////////////////////////////////////////
    //                 Runtime Phase                 //
    ///////////////////////////////////////////////////

    // Message loop, to be extended into game loop
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
            // Game update / rendering goes here
            BA::g_renderer->BeginFrame();
            BA::g_renderer->EndFrame();
        }
    }

    ////////////////////////////////////////////////////
    //                 Shutdown Phase                 //
    ////////////////////////////////////////////////////

    BA::g_renderer->Shutdown();
    BA::g_window->Shutdown();

    if (BA::g_logger->Shutdown() == false)
    {
        BA_CRASH();
    }
    delete BA::g_logger;

    return static_cast<int>(msg.wParam);
}
