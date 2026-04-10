# Game Object — Design

> **Note**: This spec documents already-implemented code.

## Architecture

The game object module has two components: a data struct (GameObject) and a manager
class (Scene). Scene owns all GameObjects in a flat vector and provides CRUD
operations.

```
Scene (manager)
 └── std::vector<GameObject>
      └── GameObject
           ├── id (uint32_t)
           ├── color (float[4])
           └── Transform
```

## Key Decisions

1. **Struct over class for GameObject**: GameObject is pure data with no behavior.
   Using a struct with public fields keeps it simple and allows direct access from
   rendering and UI code.

2. **Flat vector storage**: GameObjects are stored in a `std::vector` for simplicity.
   Destruction uses find-and-erase, which is O(n) but adequate for the current scale.

3. **Monotonic ID counter**: `m_nextId` starts at 1 and increments on each creation.
   IDs are never reused. This keeps ID assignment trivial and guarantees uniqueness.

4. **Color palette as constexpr array**: An 8-color palette defined as a `constexpr`
   local array. Objects cycle through it using `index % kPaletteSize`.

5. **Grid placement by creation index**: Position is computed from the object's index
   in the vector at creation time, not from the ID. This means grid positions are
   determined by the number of existing objects, not by ID value.

6. **Assert on invalid destroy**: `DestroyGameObject` asserts that the ID exists
   rather than silently ignoring invalid IDs. This follows the project's fail-fast
   philosophy.

## Data Structures

### GameObject (struct)

```cpp
namespace BA
{
struct GameObject
{
    uint32_t id = 0;
    float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    Transform transform;
};
}
```

### Scene (class)

```cpp
namespace BA
{
class Scene
{
public:
    void Initialize();
    void Shutdown();
    uint32_t CreateGameObject();
    void DestroyGameObject(uint32_t id);
    std::span<const GameObject> GetGameObjects() const;

private:
    std::vector<GameObject> m_gameObjects;
    uint32_t m_nextId = 1;
};

extern std::unique_ptr<Scene> g_scene;
}
```

### Constants (file-scope in Scene.cpp)

```cpp
constexpr float kPalette[][4] = { /* 8 RGBA colors */ };
constexpr uint32_t kPaletteSize = _countof(kPalette);
constexpr uint32_t kGridColumns = 5;
constexpr float kGridStartX = -0.7f;
constexpr float kGridStartY = 0.5f;
constexpr float kGridSpacing = 0.35f;
```

## Interfaces

### Public API

| Method | Signature | Description |
|--------|-----------|-------------|
| Initialize | `void Initialize()` | Logs initialization |
| Shutdown | `void Shutdown()` | Clears all game objects |
| CreateGameObject | `uint32_t CreateGameObject()` | Creates object with auto ID, palette color, grid position |
| DestroyGameObject | `void DestroyGameObject(uint32_t id)` | Finds and erases object by ID (asserts existence) |
| GetGameObjects | `std::span<const GameObject> GetGameObjects() const` | Returns read-only span of all objects |

## Cross-Module Dependencies

**Depended on by**:
- `Graphics/scene-rendering` — iterates GameObjects to render each one
- `Editor/debug-ui` — displays GameObjects in Hierarchy/Inspector panels
- `Core/window` — calls CreateGameObject/DestroyGameObject on keyboard input

**Depends on**:
- `Math/transform` — Transform struct as a GameObject member
- `Core/diagnostics` — BA_ASSERT, BA_LOG_INFO

## File Plan

All files already exist. No files to create or modify.

| File | Role |
|------|------|
| `Application/Scene/GameObject.h` | GameObject struct definition |
| `Application/Scene/Scene.h` | Scene class declaration |
| `Application/Scene/Scene.cpp` | Scene class implementation with palette and grid constants |
