# Window

> **Note**: This spec documents already-implemented code.

## Context

The window module provides the Win32 application window for BloodArena. It handles
window class registration, creation, message routing, and basic keyboard input. It
also supports injecting an external WndProc callback so the editor UI (ImGui) can
intercept input messages before the default handler processes them.

This module is the bridge between the OS and the rest of the engine — other modules
depend on the HWND it produces.

## Functional Requirements

1. **Window Class Registration**: Register a Win32 window class named
   `"BloodArenaClass"` with horizontal and vertical redraw styles and an arrow cursor.

2. **Client Area Size**: The window must have a 1280x720 client area. The actual
   window size must be computed via `AdjustWindowRect` to account for decorations.

3. **Window Creation**: Create a `WS_OVERLAPPEDWINDOW` style window titled
   `"Blood Arena"` at the OS default position.

4. **Window Display**: Show the window using the `nCmdShow` parameter passed from
   `WinMain`.

5. **WM_DESTROY Handling**: On `WM_DESTROY`, post `WM_QUIT` to terminate the message
   loop.

6. **Keyboard Input — Create Object**: Pressing `'1'` (first press only, not
   auto-repeat) must create a new game object via `g_scene->CreateGameObject()`.

7. **Keyboard Input — Delete Object**: Pressing `'2'` (first press only) must destroy
   the last game object in the scene, if any exist.

8. **Editor WndProc Injection**: An external WndProc callback must be injectable via
   `SetEditorWndProc`. When set, the injected callback is invoked first for every
   message. If it returns nonzero (message consumed), the default handler is skipped.

9. **HWND Access**: The window handle must be retrievable via `GetHandle()`.

10. **Graceful Shutdown**: On shutdown, the window must be destroyed and the window
    class unregistered.

## Non-Functional Requirements

1. All Win32 API failures during initialization (RegisterClassEx, AdjustWindowRect,
   CreateWindowEx) must crash via `BA_CRASH_IF_FAILED`.

2. Auto-repeat key presses must be filtered out — only initial key-down events are
   processed.

## Out of Scope

- Window resizing or dynamic resolution changes.
- Fullscreen or borderless window modes.
- Mouse input handling.
- Multiple window support.

## Dependencies

- **Specs**: `Core/diagnostics` (BA_ASSERT, BA_CRASH_IF_FAILED, BA_LOG_INFO)
- **Modules**: `Scene/Scene` (g_scene for keyboard input — creates/destroys
  GameObjects)
- **Platform**: Windows SDK (RegisterClassEx, CreateWindowEx, AdjustWindowRect, etc.)

## Acceptance Criteria

1. Application window opens with a 1280x720 client area and title "Blood Arena".
2. Pressing '1' creates a game object (visible on screen).
3. Pressing '2' removes the last game object.
4. Holding '1' or '2' does not repeat the action (auto-repeat filtered).
5. Closing the window terminates the application cleanly.
6. An editor WndProc callback set via `SetEditorWndProc` receives messages before the
   default handler.
