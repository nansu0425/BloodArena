#include "Core/PCH.h"
#include "Core/Window.h"

namespace BA
{

void Window::Initialize(HINSTANCE hInstance, int nCmdShow)
{
    m_hInstance = hInstance;

    RegisterWindowClass();
    SetWindowRect();
    CreateWnd();
    ShowWindow(m_handle, nCmdShow);

    BA_LOG_INFO("Window initialized.");
}

void Window::Shutdown()
{
    UnregisterClass(kClassName, m_hInstance);

    BA_LOG_INFO("Window shutdown.");
}

HWND Window::GetHandle() const
{
    return m_handle;
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Window::RegisterWindowClass()
{
    WNDCLASSEX wcx = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = WndProc,
        .hInstance = m_hInstance,
        .hCursor = LoadCursor(nullptr, IDC_ARROW),
        .lpszClassName = kClassName
    };

    if (RegisterClassEx(&wcx) == 0)
    {
        BA_CRASH_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
    }
}

void Window::SetWindowRect()
{
    // Desired client area size
    m_windowRect = {
        .left = 0,
        .top = 0,
        .right = 1280,
        .bottom = 720
    };

    // Calculate the window size required to achieve the desired client area
    if (AdjustWindowRect(&m_windowRect, WS_OVERLAPPEDWINDOW, FALSE) == FALSE)
    {
        BA_CRASH_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
    }
}

void Window::CreateWnd()
{
    m_handle = CreateWindowEx(
        0,
        kClassName,
        L"Blood Arena",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        m_windowRect.right - m_windowRect.left,
        m_windowRect.bottom - m_windowRect.top,
        nullptr,
        nullptr,
        m_hInstance,
        nullptr
    );

    if (m_handle == nullptr)
    {
        BA_CRASH_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
    }
}

std::unique_ptr<Window> g_window;


} // namespace BA
