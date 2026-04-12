#pragma once

namespace BA
{

#ifdef BA_EDITOR
using WndProcCallback = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);
#endif // BA_EDITOR
using ResizeCallback = void(*)(UINT width, UINT height);

class Window
{
public:
    void Initialize(HINSTANCE hInstance, int nCmdShow);
    void Shutdown();

    HWND GetHandle() const;

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
    RECT m_windowRect = {};
    HWND m_handle = nullptr;
#ifdef BA_EDITOR
    WndProcCallback m_editorWndProc = nullptr;
#endif // BA_EDITOR
    ResizeCallback m_resizeCallback = nullptr;
};

extern std::unique_ptr<Window> g_window;

} // namespace BA
