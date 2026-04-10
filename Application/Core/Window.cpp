#include "Core/PCH.h"
#include "Core/Window.h"
#include "Scene/Scene.h"

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
    if (m_handle)
    {
        DestroyWindow(m_handle);
        m_handle = nullptr;
    }

    UnregisterClass(kClassName, m_hInstance);

    BA_LOG_INFO("Window shutdown.");
}

HWND Window::GetHandle() const
{
    return m_handle;
}

void Window::SetEditorWndProc(WndProcCallback callback)
{
    m_editorWndProc = callback;
}

void Window::SetResizeCallback(ResizeCallback callback)
{
    m_resizeCallback = callback;
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BA_ASSERT(g_window);

    // Editor WndProc returns nonzero if it consumed the message, 0 if not
    WndProcCallback editorWndProc = g_window->m_editorWndProc;
    if (editorWndProc && editorWndProc(hWnd, uMsg, wParam, lParam))
    {
        return 0;
    }

    switch (uMsg)
    {
    case WM_SIZE:
    {
        if (wParam == SIZE_MINIMIZED)
        {
            break;
        }

        ResizeCallback resizeCallback = g_window->m_resizeCallback;
        if (resizeCallback)
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            resizeCallback(width, height);
        }

        return 0;
    }
    case WM_DESTROY:
    {
        g_window->m_handle = nullptr;
        PostQuitMessage(0);
        return 0;
    }
    case WM_KEYDOWN:
    {
        bool wasAlreadyDown = (lParam >> 30) & 1;
        if (wasAlreadyDown)
        {
            break;
        }

        if (wParam == '1')
        {
            g_scene->CreateGameObject();
        }
        else if (wParam == '2')
        {
            std::span<const GameObject> gameObjects = g_scene->GetGameObjects();
            if (gameObjects.empty() == false)
            {
                g_scene->DestroyGameObject(gameObjects.back().id);
            }
        }
        return 0;
    }
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
