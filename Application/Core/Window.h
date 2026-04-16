#pragma once

namespace BA
{

struct WindowSettings
{
    int positionX = CW_USEDEFAULT;
    int positionY = CW_USEDEFAULT;
    int clientWidth = 1280;
    int clientHeight = 720;
    bool isMaximized = false;
};

#ifdef BA_EDITOR
using WndProcCallback = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);
#endif // BA_EDITOR
using ResizeCallback = void(*)(UINT width, UINT height);

class Window
{
public:
    void Initialize(HINSTANCE hInstance, int nCmdShow, const WindowSettings& settings);
    void Shutdown();

    HWND GetHandle() const;
    WindowSettings GetSettings() const;

#ifdef BA_EDITOR
    void SetEditorWndProc(WndProcCallback callback);
#endif // BA_EDITOR
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
    WindowSettings m_settings;
    RECT m_windowRect = {};
    HWND m_handle = nullptr;
#ifdef BA_EDITOR
    WndProcCallback m_editorWndProc = nullptr;
#endif // BA_EDITOR
    ResizeCallback m_resizeCallback = nullptr;
};

extern std::unique_ptr<Window> g_window;

} // namespace BA
