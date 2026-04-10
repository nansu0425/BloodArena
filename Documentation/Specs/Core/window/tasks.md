# Window — Tasks

> **Note**: This spec documents already-implemented code. All tasks are marked [done].

## Task 1: [done] Create Window class with Win32 registration and creation

**Files:** `Application/Core/Window.h`, `Application/Core/Window.cpp`

**Description:** Define a `Window` class in the `BA` namespace with Initialize and
Shutdown methods. Initialize registers a window class (`"BloodArenaClass"`,
CS_HREDRAW | CS_VREDRAW, arrow cursor), computes the window rect for a 1280x720
client area using `AdjustWindowRect`, creates a `WS_OVERLAPPEDWINDOW` window titled
`"Blood Arena"`, and shows it. Shutdown destroys the window and unregisters the
class. Declare `g_window` as `extern std::unique_ptr<Window>`.

**Acceptance Criteria:**
- Window opens with a 1280x720 client area
- Window title is "Blood Arena"
- Window is visible after Initialize
- Shutdown destroys the window and unregisters the class
- Win32 API failures crash via BA_CRASH_IF_FAILED

---

## Task 2: [done] Implement WndProc with WM_DESTROY handling

**Depends on:** Task 1

**Files:** `Application/Core/Window.cpp`

**Description:** Implement a static `WndProc` callback. Handle `WM_DESTROY` by
nulling the handle and calling `PostQuitMessage(0)`. Delegate all other messages
to `DefWindowProc`.

**Acceptance Criteria:**
- Closing the window posts WM_QUIT and terminates the application
- Unhandled messages are passed to DefWindowProc

---

## Task 3: [done] Add keyboard input for object creation and deletion

**Depends on:** Task 2

**Files:** `Application/Core/Window.cpp`

**Description:** Handle `WM_KEYDOWN` in WndProc. Filter auto-repeat presses using
lParam bit 30. On `'1'`, call `g_scene->CreateGameObject()`. On `'2'`, destroy the
last game object if the scene is not empty.

**Acceptance Criteria:**
- Pressing '1' creates a game object
- Pressing '2' destroys the last game object
- Holding a key does not repeat the action (auto-repeat filtered)
- Pressing '2' on an empty scene does nothing

---

## Task 4: [done] Add editor WndProc callback injection

**Depends on:** Task 2

**Files:** `Application/Core/Window.h`, `Application/Core/Window.cpp`

**Description:** Define `WndProcCallback` type alias. Add `SetEditorWndProc` method
that stores a callback pointer. In WndProc, invoke the editor callback first for
every message. If it returns nonzero (message consumed), skip the default handler
and return 0.

**Acceptance Criteria:**
- SetEditorWndProc stores the callback
- Editor callback receives all messages before the default handler
- If editor callback returns nonzero, the default handler is skipped
- If no editor callback is set, the default handler processes all messages normally
