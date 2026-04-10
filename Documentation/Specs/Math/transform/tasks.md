# Transform — Tasks

> **Note**: This spec documents already-implemented code. All tasks are marked [done].

## Task 1: [done] Define Transform struct

**Files:** `Application/Math/Transform.h`

**Description:** Define a `Transform` struct in the `BA` namespace with three fields:
`position` (float[3], default 0,0,0), `rotation` (float, default 0, left-handed
Z-axis in radians), and `scale` (float[3], default 1,1,1). All fields use in-class
default initializers.

**Acceptance Criteria:**
- Transform is a struct with position, rotation, and scale fields
- Default-constructed Transform represents identity (origin, no rotation, uniform scale)
- Fields use camelCase (no m_ prefix, per struct convention)

---

## Task 2: [done] Implement BuildWorldMatrix

**Depends on:** Task 1

**Files:** `Application/Math/MathUtils.h`, `Application/Math/MathUtils.cpp`

**Description:** Declare and implement `BuildWorldMatrix(const Transform&, float[4][4])`
as a free function in the `BA` namespace. Compute a row-major 4x4 world matrix using
SRT order: embed scale into basis vectors, apply Z-axis rotation (cos/sin) to X and Y
axes, and place translation in row 3 with w=1.

**Acceptance Criteria:**
- Default Transform produces an identity matrix
- Position is correctly placed in row 3 (tx, ty, tz, 1)
- Scale is applied to basis vector magnitudes
- Rotation applies cos/sin to X and Y basis vectors
- Z-axis basis vector is scaled but not rotated
