# Transform

> **Note**: This spec documents already-implemented code.

## Context

The transform module defines how objects are positioned, rotated, and scaled in the
game world. It provides a Transform data structure and a function to build a 4x4 world
matrix from it. The world matrix is consumed by the rendering pipeline to place objects
on screen.

## Functional Requirements

1. **Transform Data**: Each game object must have a transform consisting of:
   - Position: 3D float array (x, y, z), default (0, 0, 0)
   - Rotation: single float representing left-handed Z-axis rotation in radians,
     default 0
   - Scale: 3D float array (x, y, z), default (1, 1, 1)

2. **World Matrix Construction**: A function must build a 4x4 row-major world matrix
   from a Transform by applying scale, then rotation (Z-axis), then translation
   (SRT order).

3. **Matrix Output**: The world matrix must be output to a caller-provided
   `float[4][4]` array.

## Non-Functional Requirements

1. The Transform struct must use plain float arrays (no math library dependency).
2. BuildWorldMatrix must be a free function, not a method on Transform.

## Out of Scope

- Full 3D rotation (pitch/yaw) — only Z-axis rotation is supported.
- Matrix multiplication utilities.
- View or projection matrices.
- Quaternion representation.

## Dependencies

- **Specs**: None (pure math, no runtime dependencies on other modules).
- **Used by**: `Scene/game-object` (Transform as component),
  `Graphics/scene-rendering` (BuildWorldMatrix for rendering).

## Acceptance Criteria

1. A default Transform produces an identity world matrix.
2. A Transform with position (1, 2, 0) produces a matrix with translation in row 3.
3. A Transform with non-uniform scale produces correctly scaled basis vectors.
4. A Transform with rotation produces correct cos/sin values in the X and Y basis
   vectors.
