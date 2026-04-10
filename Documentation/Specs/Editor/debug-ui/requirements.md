# Debug UI

> **Note**: This spec documents already-implemented code.

## Context

The debug UI module provides an ImGui-based editor overlay for BloodArena. It renders
docked panels over the game viewport, allowing developers to inspect game objects and
their properties. The UI uses ImGui's docking system with a passthrough central node
so the game scene remains visible behind the panels.

## Functional Requirements

1. **ImGui Initialization**: Initialize an ImGui context with keyboard navigation and
   docking enabled, using the dark color scheme.

2. **Platform Backends**: Initialize ImGui Win32 and DirectX 11 backends using the
   application window handle and D3D11 device/context.

3. **Input Forwarding**: Register an ImGui WndProc handler with the Window module via
   `SetEditorWndProc` so ImGui receives Win32 input messages.

4. **Docking**: Enable a full-viewport dockspace with `ImGuiDockNodeFlags_PassthruCentralNode`
   so the game scene is visible through the central area.

5. **Hierarchy Panel**: Display a "Hierarchy" window listing all game objects. Each
   entry shows `"GameObject {id}"` and is selectable. Clicking an entry selects that
   object.

6. **Inspector Panel**: Display an "Inspector" window showing the selected object's
   properties:
   - ID
   - Position (x, y, z)
   - Rotation (single float)
   - Scale (x, y, z)
   - Color (swatch + RGBA values)

7. **No Selection State**: When no object is selected (id = 0), the Inspector must
   show "No object selected" as disabled text.

8. **Stale Selection Handling**: If the selected object no longer exists in the scene,
   the selection must be cleared and the Inspector must show "No object selected".

9. **Graceful Shutdown**: Shutdown must destroy ImGui backends and context in the
   correct order (DX11 → Win32 → context).

## Non-Functional Requirements

1. ImGui headers must be included with warnings suppressed (`#pragma warning(push, 0)`).
2. All properties in the Inspector are read-only (display only, no editing).

## Out of Scope

- Property editing (drag/input fields for transform values).
- Object creation or deletion from the UI.
- Viewport selection (clicking objects in the game view).
- Multiple viewports or detachable panels.
- Custom ImGui themes or styles beyond the default dark theme.

## Dependencies

- **Specs**: `Core/window` (HWND for ImGui Win32 backend, SetEditorWndProc)
- **Specs**: `Graphics/graphics-device` (Device/Context for ImGui DX11 backend)
- **Specs**: `Scene/game-object` (iterating GameObjects for display and selection)
- **Specs**: `Core/diagnostics` (BA_ASSERT, BA_LOG_INFO)
- **External**: imgui (vcpkg, with win32-binding, dx11-binding, docking-experimental)

## Acceptance Criteria

1. Docked panels appear over the game viewport; game scene is visible in the central
   area.
2. Hierarchy panel lists all GameObjects by ID.
3. Clicking a GameObject in the Hierarchy selects it.
4. Inspector displays the selected object's ID, transform, and color.
5. Deleting the selected object clears the selection and shows "No object selected".
6. ImGui receives keyboard and mouse input correctly.
