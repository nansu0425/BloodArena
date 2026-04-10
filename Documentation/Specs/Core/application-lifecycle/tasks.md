# Application Lifecycle — Tasks

> **Note**: This spec documents already-implemented code. All tasks are marked [done].

## Task 1: [done] Create WinMain entry point

**Files:** `Application/Core/WinMain.cpp`

**Description:** Implement `wWinMain` as the Win32 entry point. It calls
`BA::Initialize(hInstance, nShowCmd)`, `BA::Run()`, and `BA::Shutdown()` in sequence,
returning the exit code from Run.

**Acceptance Criteria:**
- wWinMain calls Initialize, Run, Shutdown in order
- Exit code from Run is returned as the process exit code
- Unused parameters (hPrevInstance, lpCmdLine) are unnamed

---

## Task 2: [done] Implement Initialize with subsystem creation in dependency order

**Depends on:** Task 1

**Files:** `Application/Core/Lifecycle.h`, `Application/Core/Lifecycle.cpp`

**Description:** Declare Initialize, Run, and Shutdown as free functions in the `BA`
namespace. Implement `Initialize()` which enables CRT memory leak detection in debug
builds, then creates and initializes all subsystems via `make_unique` in dependency
order: Logger → Window → GraphicsDevice → Scene → SceneRenderer → EditorUI.

**Acceptance Criteria:**
- CRT leak detection is enabled before any allocations (debug only)
- All six subsystems are created and initialized in the correct order
- GraphicsDevice receives the HWND from Window
- No subsystem depends on one initialized after it

---

## Task 3: [done] Implement Run loop and Shutdown

**Depends on:** Task 2

**Files:** `Application/Core/Lifecycle.cpp`

**Description:** Implement `Run()` with a `PeekMessage` loop that processes messages
when available and renders frames (BeginFrame → SceneRenderer::Render →
EditorUI::Render → EndFrame) when idle. Exit on `WM_QUIT` and return `msg.wParam`.
Implement `Shutdown()` which calls Shutdown() + reset() on each subsystem in reverse
initialization order.

**Acceptance Criteria:**
- Messages are processed without blocking
- Frames render during idle time
- Loop exits cleanly on WM_QUIT
- All subsystems are shut down and released in reverse order
- No resource leaks after shutdown
