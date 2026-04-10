# Debug UI — Design

> **Note**: This spec documents already-implemented code.

## Architecture

EditorUI renders after the scene in each frame. It uses ImGui's immediate mode API
to build the UI each frame, with docking enabled for panel layout.

```
GraphicsDevice::BeginFrame()
 └── SceneRenderer::Render()
      └── EditorUI::Render()
           ├── ImGui NewFrame (DX11, Win32, ImGui)
           ├── DockSpaceOverViewport (passthrough central node)
           ├── RenderHierarchy()
           ├── RenderInspector()
           └── ImGui Render + DX11 RenderDrawData
```

Input flows through the Window's WndProc callback chain:

```
Win32 message → Window::WndProc
                 ├── ImGuiWndProcHandler (first) → ImGui input state
                 └── Default handler (if not consumed)
```

## Key Decisions

1. **Static bridge function for WndProc**: `ImGuiWndProcHandler` is a file-scope
   static function that forwards to `ImGui_ImplWin32_WndProcHandler`. This bridges
   the `WndProcCallback` type expected by Window with ImGui's extern declaration.

2. **Passthrough docking**: `ImGuiDockNodeFlags_PassthruCentralNode` makes the
   dockspace's central area transparent, so the game scene renders behind the docked
   panels without a separate viewport.

3. **Selection by ID**: `m_selectedGameObjectId` stores the selected object's ID
   (not index or pointer). ID 0 means no selection. This is robust against vector
   reordering from object deletion.

4. **Stale selection auto-clear**: If the selected ID is not found in the scene's
   GameObjects, `m_selectedGameObjectId` is reset to 0. This handles the case where
   a selected object is destroyed.

5. **Read-only Inspector**: Properties are displayed with `ImGui::Text` rather than
   input widgets. Editing is planned for a future spec.

6. **Warning suppression for ImGui headers**: ImGui headers generate warnings at
   warning level 4. `#pragma warning(push, 0)` suppresses them to keep the project's
   warnings-as-errors policy clean.

## Data Structures

### EditorUI (class)

```cpp
namespace BA
{
class EditorUI
{
public:
    void Initialize();
    void Shutdown();
    void Render();

private:
    void RenderHierarchy();
    void RenderInspector();

    uint32_t m_selectedGameObjectId = 0;
};

extern std::unique_ptr<EditorUI> g_editorUI;
}
```

### ImGuiWndProcHandler (file-scope static)

```cpp
static LRESULT ImGuiWndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
}
```

## Interfaces

### Public API

| Method | Signature | Description |
|--------|-----------|-------------|
| Initialize | `void Initialize()` | Creates ImGui context, inits backends, registers WndProc |
| Shutdown | `void Shutdown()` | Shuts down backends, destroys context |
| Render | `void Render()` | Runs full ImGui frame: NewFrame, panels, Render, DrawData |

### Private Methods

| Method | Description |
|--------|-------------|
| RenderHierarchy | Lists all GameObjects as Selectable items, updates selection |
| RenderInspector | Displays selected object's ID, transform, color (read-only) |

## Cross-Module Dependencies

**Depended on by**:
- `Core/application-lifecycle` — calls Render() in the frame loop

**Depends on**:
- `Core/window` — HWND for ImGui Win32 init, SetEditorWndProc for input
- `Graphics/graphics-device` — Device/Context for ImGui DX11 init
- `Scene/game-object` — g_scene->GetGameObjects() for panel content
- `Core/diagnostics` — BA_ASSERT, BA_LOG_INFO

## File Plan

All files already exist. No files to create or modify.

| File | Role |
|------|------|
| `Application/Editor/EditorUI.h` | EditorUI class declaration |
| `Application/Editor/EditorUI.cpp` | EditorUI implementation with ImGui integration |
