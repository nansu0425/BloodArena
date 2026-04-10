# Game Object

> **Note**: This spec documents already-implemented code.

## Context

The game object module defines the core runtime entity of BloodArena. A GameObject is
a lightweight data container with an ID, color, and transform. The Scene class manages
a collection of GameObjects, providing creation, destruction, and query operations.

This module is consumed by the rendering pipeline (to draw objects) and the editor UI
(to display and select objects).

## Functional Requirements

1. **GameObject Data**: A game object must have:
   - A unique ID (`uint32_t`), default 0
   - A color (`float[4]` RGBA), default white (1, 1, 1, 1)
   - A Transform component

2. **Unique ID Assignment**: Each created game object must receive a unique,
   monotonically increasing ID starting from 1.

3. **Runtime Creation**: Game objects must be creatable at runtime via
   `CreateGameObject()`, which returns the assigned ID.

4. **Automatic Color Assignment**: New objects must be assigned a color from a
   rotating 8-color palette based on their creation index.

5. **Automatic Grid Placement**: New objects must be placed on a grid layout:
   - 5 columns
   - Starting position: (-0.7, 0.5)
   - Spacing: 0.35 in both X and Y
   - Wraps to the next row after 5 columns

6. **Runtime Destruction**: Game objects must be destroyable by ID via
   `DestroyGameObject(uint32_t id)`. Destroying a nonexistent ID is an assertion
   failure.

7. **Query**: The full list of game objects must be queryable as a read-only span
   via `GetGameObjects()`.

8. **Logging**: Creation and destruction must log the affected object's ID at info
   level.

## Non-Functional Requirements

1. GameObject must be a struct (plain data, no methods).
2. Scene must assert that the ID exists before destroying.

## Out of Scope

- Component system or entity-component-system architecture.
- Serialization or persistence of game objects.
- Object naming or tagging.
- Hierarchical parent-child relationships.

## Dependencies

- **Specs**: `Math/transform` (Transform struct as component)
- **Specs**: `Core/diagnostics` (BA_ASSERT, BA_LOG_INFO)

## Acceptance Criteria

1. `CreateGameObject()` returns a unique ID each time.
2. Created objects appear with palette colors cycling through 8 colors.
3. Created objects are positioned on a 5-column grid starting at (-0.7, 0.5).
4. `DestroyGameObject(id)` removes the correct object from the scene.
5. `GetGameObjects()` returns a span containing all live objects.
6. Destroying a nonexistent ID triggers BA_ASSERT in debug builds.
