# Game Object — Tasks

> **Note**: This spec documents already-implemented code. All tasks are marked [done].

## Task 1: [done] Define GameObject struct

**Files:** `Application/Scene/GameObject.h`

**Description:** Define a `GameObject` struct in the `BA` namespace with three fields:
`id` (uint32_t, default 0), `color` (float[4], default white), and `transform`
(Transform, using Transform's defaults). Include `Math/Transform.h`.

**Acceptance Criteria:**
- GameObject is a struct with id, color, and transform fields
- Fields use camelCase (no m_ prefix, per struct convention)
- Default-constructed GameObject has id=0, white color, identity transform

---

## Task 2: [done] Implement Scene with CreateGameObject and palette/grid placement

**Depends on:** Task 1

**Files:** `Application/Scene/Scene.h`, `Application/Scene/Scene.cpp`

**Description:** Define a `Scene` class with Initialize, Shutdown, CreateGameObject,
DestroyGameObject, and GetGameObjects methods. Implement `CreateGameObject()` which
assigns a monotonically increasing ID (starting from 1), a color from an 8-color
palette (cycling by index), and a position on a 5-column grid (start: -0.7, 0.5;
spacing: 0.35). Log creation at info level. Declare `g_scene` as
`extern std::unique_ptr<Scene>`.

**Acceptance Criteria:**
- IDs are unique and start at 1
- Colors cycle through the 8-color palette
- Positions follow the 5-column grid layout
- Creation is logged with the object's ID

---

## Task 3: [done] Implement DestroyGameObject

**Depends on:** Task 2

**Files:** `Application/Scene/Scene.cpp`

**Description:** Implement `DestroyGameObject(uint32_t id)` which finds the object
by ID using `std::find_if` and erases it from the vector. Assert that the object
exists before erasing. Log destruction at info level.

**Acceptance Criteria:**
- Correct object is removed by ID
- Assertion fires if ID does not exist
- Destruction is logged with the object's ID
- `GetGameObjects()` no longer includes the destroyed object
