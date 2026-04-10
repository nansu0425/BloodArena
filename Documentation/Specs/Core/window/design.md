# Window — Design

> **Note**: This spec documents already-implemented code.

## Architecture

The Window module sits between the OS and the engine. It produces an HWND that
GraphicsDevice uses for swap chain creation, and routes Win32 messages to both the
editor UI and the default game input handler.

```
OS (Win32 messages)
 └── Window::WndProc (static)
      ├── Editor WndProc callback (if set, gets first look)
      └── Default handler
           ├── WM_DESTROY → PostQuitMessage
           └── WM_KEYDOWN → Scene create/destroy
```

## Key Decisions

1. **Static WndProc with global access**: `WndProc` is a static method that accesses
   `g_window` to reach instance members. This is the standard Win32 pattern since
   the OS callback cannot receive a `this` pointer directly.

2. **Editor callback injection over subclassing**: `SetEditorWndProc` stores a
   function pointer that is called before the default handler. This avoids Win32
   window subclassing complexity and gives the editor full priority over input.

3. **Auto-repeat filtering via lParam bit 30**: The previous key state bit (bit 30
   of lParam) distinguishes initial presses from auto-repeats, preventing repeated
   object creation/deletion from held keys.

4. **Crash on initialization failure**: All Win32 API calls that can fail
   (RegisterClassEx, AdjustWindowRect, CreateWindowEx) use `BA_CRASH_IF_FAILED`
   with `HRESULT_FROM_WIN32(GetLastError())`. The application cannot function
   without a window.

5. **Window class name as constexpr**: `kClassName` is a `static constexpr LPCWSTR`
   used for both registration and unregistration, ensuring consistency.

## Data Structures

### WndProcCallback (type alias)

```cpp
using WndProcCallback = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);
```

A function pointer type matching the Win32 `WNDPROC` signature.

### Window (class)

```cpp
namespace BA
{
class Window
{
public:
    void Initialize(HINSTANCE hInstance, int nCmdShow);
    void Shutdown();
    HWND GetHandle() const;
    void SetEditorWndProc(WndProcCallback callback);

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void RegisterWindowClass();
    void SetWindowRect();
    void CreateWnd();

    static constexpr LPCWSTR kClassName = L"BloodArenaClass";

    HINSTANCE m_hInstance = nullptr;
    RECT m_windowRect = {};
    HWND m_handle = nullptr;
    WndProcCallback m_editorWndProc = nullptr;
};

extern std::unique_ptr<Window> g_window;
}
```

## Interfaces

### Public API

| Method | Signature | Description |
|--------|-----------|-------------|
| Initialize | `void Initialize(HINSTANCE hInstance, int nCmdShow)` | Registers class, computes rect, creates and shows window |
| Shutdown | `void Shutdown()` | Destroys window and unregisters class |
| GetHandle | `HWND GetHandle() const` | Returns the window handle |
| SetEditorWndProc | `void SetEditorWndProc(WndProcCallback callback)` | Sets editor input callback |

### Private Methods

| Method | Description |
|--------|-------------|
| WndProc | Static callback: editor dispatch → WM_DESTROY/WM_KEYDOWN → DefWindowProc |
| RegisterWindowClass | Registers `"BloodArenaClass"` with CS_HREDRAW \| CS_VREDRAW |
| SetWindowRect | Computes window rect for 1280x720 client area via AdjustWindowRect |
| CreateWnd | Creates the WS_OVERLAPPEDWINDOW window |

## Cross-Module Dependencies

**Depended on by**:
- `Graphics/graphics-device` — needs HWND for swap chain creation
- `Editor/debug-ui` — injects WndProc callback for ImGui input
- `Core/application-lifecycle` — creates and shuts down the window

**Depends on**:
- `Core/diagnostics` — BA_ASSERT, BA_CRASH_IF_FAILED, BA_LOG_INFO
- `Scene/game-object` — g_scene for keyboard-driven create/destroy

## File Plan

All files already exist. No files to create or modify.

| File | Role |
|------|------|
| `Application/Core/Window.h` | Window class declaration, WndProcCallback type alias |
| `Application/Core/Window.cpp` | Window class implementation |
