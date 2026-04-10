#pragma once

namespace BA
{

using WndProcCallback = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);
using ResizeCallback = void(*)(UINT width, UINT height);

class Window
{
public:
    void Initialize(HINSTANCE hInstance, int nCmdShow);
    void Shutdown();

    HWND GetHandle() const;

    void SetEditorWndProc(WndProcCallback callback);
    void SetResizeCallback(ResizeCallback callback);

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void RegisterWindowClass();
    void SetWindowRect();
    void CreateWnd();

private:
    static constexpr LPCWSTR kClassName = L"BloodArenaClass";

private:
    HINSTANCE m_hInstance = nullptr;
    RECT m_windowRect = {};
    HWND m_handle = nullptr;
    WndProcCallback m_editorWndProc = nullptr;
    ResizeCallback m_resizeCallback = nullptr;
};

extern std::unique_ptr<Window> g_window;

} // namespace BA
