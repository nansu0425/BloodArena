# Application Lifecycle — Design

> **Note**: This spec documents already-implemented code.

## Architecture

The lifecycle module is the top of the call graph. WinMain calls three free functions
in the `BA` namespace that orchestrate the entire application.

```
wWinMain
 ├── BA::Initialize(hInstance, nShowCmd)
 │    ├── CRT debug setup (debug only)
 │    ├── Logger::Initialize()
 │    ├── Window::Initialize()
 │    ├── GraphicsDevice::Initialize()
 │    ├── Scene::Initialize()
 │    ├── SceneRenderer::Initialize()
 │    └── EditorUI::Initialize()
 │
 ├── BA::Run()
 │    └── while (msg != WM_QUIT)
 │         ├── PeekMessage → Translate/Dispatch
 │         └── else: BeginFrame → Render → Render → EndFrame
 │
 └── BA::Shutdown()
      ├── EditorUI::Shutdown() + reset
      ├── SceneRenderer::Shutdown() + reset
      ├── Scene::Shutdown() + reset
      ├── GraphicsDevice::Shutdown() + reset
      ├── Window::Shutdown() + reset
      └── Logger::Shutdown() + reset
```

## Key Decisions

1. **Free functions over application class**: Initialize, Run, and Shutdown are free
   functions in the `BA` namespace rather than methods on an Application class. This
   keeps the orchestration flat and avoids an unnecessary abstraction layer.

2. **Explicit Shutdown + reset**: Each subsystem is shut down explicitly before its
   unique_ptr is reset. This ensures deterministic cleanup order rather than relying
   on destructor ordering, which is critical because subsystems have cross-dependencies
   (e.g., EditorUI must shut down before GraphicsDevice).

3. **PeekMessage over GetMessage**: `PeekMessage` is non-blocking, allowing the
   application to render frames when no messages are pending. This is the standard
   game loop pattern for responsive rendering.

4. **Render during idle only**: Frames are only rendered when there are no pending
   messages. This ensures input is processed promptly before the next frame.

5. **CRT leak detection before any allocation**: `_CrtSetDbgFlag` is called as the
   very first thing in Initialize, before even Logger creation, to catch all
   allocations.

## Data Structures

No new types are introduced. This module uses the globals declared by other modules:
`g_logger`, `g_window`, `g_graphicsDevice`, `g_scene`, `g_sceneRenderer`, `g_editorUI`.

## Interfaces

### Free Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| Initialize | `void Initialize(HINSTANCE hInstance, int nShowCmd)` | Creates and initializes all subsystems in order |
| Run | `int Run()` | Message pump + render loop, returns exit code |
| Shutdown | `void Shutdown()` | Shuts down and releases all subsystems in reverse order |

### Entry Point

| Function | Signature | Description |
|----------|-----------|-------------|
| wWinMain | `int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)` | Calls Initialize → Run → Shutdown |

## Cross-Module Dependencies

**Depended on by**: Nothing — this is the top-level orchestrator.

**Depends on**: Every other module:
- `Core/diagnostics` — g_logger, CRT debug flags
- `Core/window` — g_window
- `Graphics/graphics-device` — g_graphicsDevice (Initialize, BeginFrame, EndFrame)
- `Scene/game-object` — g_scene
- `Graphics/scene-rendering` — g_sceneRenderer (Render)
- `Editor/debug-ui` — g_editorUI (Render)

## File Plan

All files already exist. No files to create or modify.

| File | Role |
|------|------|
| `Application/Core/WinMain.cpp` | Win32 entry point (calls Initialize/Run/Shutdown) |
| `Application/Core/Lifecycle.h` | Initialize, Run, Shutdown declarations |
| `Application/Core/Lifecycle.cpp` | Initialize, Run, Shutdown implementations |
