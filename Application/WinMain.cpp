#include "PCH.h"

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

void ShowLastError(LPCWSTR errorFunction)
{
    HRESULT hr = HRESULT_FROM_WIN32(GetLastError());

    LPWSTR errorMsg = nullptr;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        hr,
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
        swprintf_s(messageBuf, L"%s failed: %s (HRESULT: 0x%08X)", errorFunction, errorMsg, hr);
        MessageBox(nullptr, messageBuf, L"Last Error", MB_OK | MB_ICONERROR);
        LocalFree(errorMsg);
    }
    else
    {
        swprintf_s(messageBuf, L"%s failed: Unknown error code (HRESULT: 0x%08X)", errorFunction, hr);
        MessageBox(nullptr, messageBuf, L"Last Error", MB_OK | MB_ICONERROR);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"BloodArenaClass";
    if (RegisterClassEx(&wc) == 0)
    {
        ShowLastError(L"RegisterClassEx");
        return -1;
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
        ShowLastError(L"CreateWindowEx");
        return -1;
    }

    ShowWindow(hWnd, nCmdShow);

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
        }
    }

    return static_cast<int>(msg.wParam);
}
