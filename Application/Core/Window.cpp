#include "Core/PCH.h"
#include "Core/Window.h"
#include "Core/Input.h"
#include "Scene/Scene.h"

#include <windowsx.h>

#ifdef BA_EDITOR
#include "Editor/EditorUI.h"
#endif // BA_EDITOR

namespace BA
{

void Window::Initialize(HINSTANCE hInstance, int nCmdShow, const WindowSettings& settings)
{
    m_hInstance = hInstance;
    m_settings = settings;

    RegisterWindowClass();
    SetWindowRect();
    CreateWnd();
    ShowWindow(m_handle, m_settings.isMaximized ? SW_MAXIMIZE : nCmdShow);

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

#ifdef BA_EDITOR
void Window::SetEditorWndProc(WndProcCallback callback)
{
    m_editorWndProc = callback;
}
#endif // BA_EDITOR

void Window::SetResizeCallback(ResizeCallback callback)
{
    m_resizeCallback = callback;
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BA_ASSERT(g_window);
    BA_ASSERT(g_input);

    switch (uMsg)
    {
    case WM_MOUSEMOVE:
        g_input->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;
    case WM_RBUTTONDOWN:
        g_input->OnRightMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        SetCapture(hWnd);
        break;
    case WM_RBUTTONUP:
        g_input->OnRightMouseUp();
        ReleaseCapture();
        break;
    case WM_KEYUP:
        g_input->OnKeyUp(static_cast<uint32_t>(wParam));
        break;
    case WM_KEYDOWN:
        if (((lParam >> 30) & 1) == 0)
        {
            g_input->OnKeyDown(static_cast<uint32_t>(wParam));
        }
        break;
    }

#ifdef BA_EDITOR
    WndProcCallback editorWndProc = g_window->m_editorWndProc;
    if (editorWndProc && editorWndProc(hWnd, uMsg, wParam, lParam))
    {
        return 0;
    }
#endif // BA_EDITOR

    switch (uMsg)
    {
    case WM_SIZE:
    {
        if (wParam == SIZE_MINIMIZED)
        {
            break;
        }

        UINT width = LOWORD(lParam);
        UINT height = HIWORD(lParam);

        g_window->m_settings.isMaximized = (wParam == SIZE_MAXIMIZED);
        g_window->m_settings.clientWidth = static_cast<int>(width);
        g_window->m_settings.clientHeight = static_cast<int>(height);

        ResizeCallback resizeCallback = g_window->m_resizeCallback;
        if (resizeCallback)
        {
            resizeCallback(width, height);
        }

        return 0;
    }
    case WM_MOVE:
    {
        g_window->m_settings.positionX = GET_X_LPARAM(lParam);
        g_window->m_settings.positionY = GET_Y_LPARAM(lParam);
        return 0;
    }
    case WM_DESTROY:
    {
        g_window->m_handle = nullptr;
        PostQuitMessage(0);
        return 0;
    }
    case WM_MOUSEMOVE:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_KEYUP:
        return 0;
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
                uint32_t id = gameObjects.back().id;
#ifdef BA_EDITOR
                if (g_editorUI->GetSelectedGameObjectId() == id)
                {
                    g_editorUI->SetSelectedGameObjectId(0);
                }
#endif // BA_EDITOR
                g_scene->DestroyGameObject(id);
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

WindowSettings Window::GetSettings() const
{
    return m_settings;
}

void Window::SetWindowRect()
{
    m_windowRect = {
        .left = 0,
        .top = 0,
        .right = static_cast<LONG>(m_settings.clientWidth),
        .bottom = static_cast<LONG>(m_settings.clientHeight)
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
        m_settings.positionX,
        m_settings.positionY,
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
