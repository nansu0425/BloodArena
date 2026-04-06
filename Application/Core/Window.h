#pragma once

namespace BA
{

class Window
{
public:
    void Initialize(HINSTANCE hInstance, int nCmdShow);
    void Shutdown();

    HWND GetHandle() const;

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
};

extern std::unique_ptr<Window> g_window;

} // namespace BA
