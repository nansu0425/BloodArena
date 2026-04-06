#include "Core/PCH.h"
#include "Graphics/Renderer.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void ShowLastError(HRESULT lastError, LPCWSTR functionName)
{
    LPWSTR errorMsg = nullptr;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        lastError,
        0,
        reinterpret_cast<LPWSTR>(&errorMsg), // Interpreted as LPWSTR* internally in ALLOCATE_BUFFER mode
        0,
        nullptr
    );

    // Message format
    // "CreateSwapChainForHwnd failed: The parameter is incorrect. (HRESULT: 0x80070057)"
    WCHAR messageBuf[256] = {};

    if (errorMsg != nullptr)
    {
        swprintf_s(messageBuf, L"%s failed: %s (HRESULT: 0x%08X)", functionName, errorMsg, lastError);
        MessageBox(nullptr, messageBuf, L"Last Error", MB_OK | MB_ICONERROR);
        LocalFree(errorMsg);
    }
    else
    {
        swprintf_s(messageBuf, L"%s failed: Unknown error code (HRESULT: 0x%08X)", functionName, lastError);
        MessageBox(nullptr, messageBuf, L"Last Error", MB_OK | MB_ICONERROR);
    }
}

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

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"BloodArenaClass";
    if (RegisterClassEx(&wc) == 0)
    {
        ShowLastError(HRESULT_FROM_WIN32(GetLastError()), L"RegisterClassEx");
        BA_CRASH();
    }

    RECT rc = {0, 0, 1280, 720};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hWnd = CreateWindowEx(
        0,
        L"BloodArenaClass",
        L"Blood Arena",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );
    if (hWnd == nullptr)
    {
        ShowLastError(HRESULT_FROM_WIN32(GetLastError()), L"CreateWindowEx");
        BA_CRASH();
    }

    ShowWindow(hWnd, nShowCmd);

    BA::g_renderer = std::make_unique<BA::Renderer>();
    BA::g_renderer->Initialize(hWnd);

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

    if (BA::g_logger->Shutdown() == false)
    {
        BA_CRASH();
    }
    delete BA::g_logger;

    return static_cast<int>(msg.wParam);
}
