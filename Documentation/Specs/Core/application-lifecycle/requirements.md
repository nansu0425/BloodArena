# Application Lifecycle

> **Note**: This spec documents already-implemented code.

## Context

The application lifecycle module is the top-level orchestrator of BloodArena. It
defines the Win32 entry point (`wWinMain`), initializes all subsystems in dependency
order, runs the main event/render loop, and shuts everything down in reverse order.

This module ties all other modules together into a running application.

## Functional Requirements

1. **Entry Point**: The application must use `wWinMain` as the Win32 entry point,
   forwarding `HINSTANCE` and `nShowCmd` to initialization.

2. **Initialization Order**: All subsystems must be initialized in dependency order:
   1. Logger
   2. Window
   3. GraphicsDevice (depends on Window's HWND)
   4. Scene
   5. SceneRenderer (depends on GraphicsDevice)
   6. EditorUI (depends on Window, GraphicsDevice)

3. **CRT Memory Leak Detection**: In debug builds, CRT memory leak detection must be
   enabled before any allocations via `_CrtSetDbgFlag` with `_CRTDBG_LEAK_CHECK_DF`.

4. **Main Loop**: The run loop must:
   - Use `PeekMessage` for non-blocking message processing
   - Process messages (TranslateMessage/DispatchMessage) when available
   - Render a frame when no messages are pending:
     BeginFrame → SceneRenderer::Render → EditorUI::Render → EndFrame

5. **Loop Termination**: The loop must exit when `WM_QUIT` is received.

6. **Exit Code**: The application must return `msg.wParam` as the process exit code.

7. **Shutdown Order**: All subsystems must be shut down in reverse initialization
   order. Each subsystem must be explicitly shut down (`Shutdown()`) then released
   (`reset()`) before the next:
   1. EditorUI
   2. SceneRenderer
   3. Scene
   4. GraphicsDevice
   5. Window
   6. Logger

## Non-Functional Requirements

1. Each subsystem is owned via a global `std::unique_ptr` and explicitly created
   with `std::make_unique` during initialization.
2. The entry point must be minimal — only call Initialize, Run, Shutdown.

## Out of Scope

- Command-line argument parsing.
- Multiple windows or application instances.
- Frame rate limiting or delta time calculation.
- Error recovery or restart logic.

## Dependencies

- **Specs**: All other specs — this module orchestrates every subsystem:
  - `Core/diagnostics` (Logger, CRT debug flags)
  - `Core/window` (Window)
  - `Graphics/graphics-device` (GraphicsDevice)
  - `Scene/game-object` (Scene)
  - `Graphics/scene-rendering` (SceneRenderer)
  - `Editor/debug-ui` (EditorUI)

## Acceptance Criteria

1. Application starts, shows a window with rendered scene and editor UI.
2. Application responds to input (keyboard creates/destroys objects, ImGui panels
   are interactive).
3. Closing the window shuts down all subsystems cleanly without crashes or leaks.
4. In debug builds, CRT reports memory leaks (if any) at process exit.
