# Debug UI — Tasks

> **Note**: This spec documents already-implemented code. All tasks are marked [done].

## Task 1: [done] Initialize ImGui with DX11 and Win32 backends

**Files:** `Application/Editor/EditorUI.h`, `Application/Editor/EditorUI.cpp`

**Description:** Define an `EditorUI` class with Initialize, Shutdown, and Render
methods. Initialize creates an ImGui context, enables keyboard navigation and docking,
applies the dark style, initializes Win32 and DX11 backends with the window handle
and D3D11 device/context, and registers the ImGui WndProc handler via
`g_window->SetEditorWndProc`. Define a file-scope static `ImGuiWndProcHandler` that
bridges to `ImGui_ImplWin32_WndProcHandler`. Shutdown tears down in reverse order
(DX11 → Win32 → context). Declare `g_editorUI` as `extern std::unique_ptr<EditorUI>`.

**Acceptance Criteria:**
- ImGui context is created with docking and keyboard navigation enabled
- Win32 and DX11 backends are initialized with correct handles
- ImGui receives input through the WndProc callback
- Shutdown destroys backends and context without errors

---

## Task 2: [done] Set up docking with passthrough central node

**Depends on:** Task 1

**Files:** `Application/Editor/EditorUI.cpp`

**Description:** In `Render()`, after calling NewFrame for all three layers (DX11,
Win32, ImGui), call `ImGui::DockSpaceOverViewport` with
`ImGuiDockNodeFlags_PassthruCentralNode` to create a full-viewport dockspace where
the game scene is visible through the central area. End the frame with
`ImGui::Render()` and `ImGui_ImplDX11_RenderDrawData`.

**Acceptance Criteria:**
- Dockspace covers the entire viewport
- Central area is transparent (game scene visible behind panels)
- Panels can be docked to edges of the viewport

---

## Task 3: [done] Implement Hierarchy panel with object listing and selection

**Depends on:** Task 2

**Files:** `Application/Editor/EditorUI.cpp`

**Description:** Implement `RenderHierarchy()` which creates a "Hierarchy" window
and iterates over all GameObjects. Each object is displayed as an `ImGui::Selectable`
with label `"GameObject {id}"`. Clicking a selectable sets
`m_selectedGameObjectId` to that object's ID.

**Acceptance Criteria:**
- Hierarchy window lists all GameObjects
- Each entry shows "GameObject {id}"
- Clicking an entry selects it (visually highlighted)
- Selection state persists across frames

---

## Task 4: [done] Implement Inspector panel with read-only property display

**Depends on:** Task 3

**Files:** `Application/Editor/EditorUI.cpp`

**Description:** Implement `RenderInspector()` which creates an "Inspector" window.
If no object is selected (id = 0), show "No object selected" as disabled text. If
the selected object no longer exists, clear selection and show the same message.
Otherwise display: ID, position (x,y,z), rotation, scale (x,y,z), and color (swatch
via `ImGui::ColorButton` + RGBA text).

**Acceptance Criteria:**
- Inspector shows "No object selected" when nothing is selected
- Inspector shows correct ID, transform, and color for the selected object
- Deleting the selected object clears the selection
- Color swatch matches the object's color values
