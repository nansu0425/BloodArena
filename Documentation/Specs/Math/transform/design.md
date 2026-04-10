# Transform — Design

> **Note**: This spec documents already-implemented code.

## Architecture

The transform module is a pure math layer with no dependencies on other application
modules. It provides data (Transform struct) and computation (BuildWorldMatrix
function) consumed by the scene and rendering modules.

```
Transform struct ──── used by ──── GameObject (Scene module)
     │
BuildWorldMatrix() ── used by ──── SceneRenderer (Graphics module)
```

## Key Decisions

1. **Struct with plain arrays**: Transform uses `float[3]` for position and scale
   instead of a vector class. This avoids introducing a math library dependency at
   this stage and keeps the data layout simple and explicit.

2. **Z-axis rotation only**: The current rendering is effectively 2D (orthographic
   triangles). A single float for Z-axis rotation is sufficient. Full 3D rotation
   will be introduced when needed.

3. **Row-major matrix layout**: The output matrix uses row-major order where row 3
   contains translation. This matches the convention used by the HLSL vertex shader
   with `row_major float4x4`.

4. **Free function over method**: `BuildWorldMatrix` is a free function rather than
   a Transform method. This keeps Transform as a pure data struct and separates data
   from computation.

5. **SRT composition order**: Scale is applied first (embedded in basis vectors),
   then rotation (2D rotation matrix applied to X/Y axes), then translation
   (placed in row 3). This is the standard SRT ordering.

## Data Structures

### Transform (struct)

```cpp
namespace BA
{
struct Transform
{
    float position[3] = {0.0f, 0.0f, 0.0f};
    float rotation    = 0.0f; // Left-handed Z-Axis, radians
    float scale[3]    = {1.0f, 1.0f, 1.0f};
};
}
```

### World Matrix Layout (row-major float[4][4])

```
Row 0: [ sx*cos,  sx*sin,  0,  0 ]   ← scaled X-axis
Row 1: [-sy*sin,  sy*cos,  0,  0 ]   ← scaled Y-axis
Row 2: [      0,       0, sz,  0 ]   ← scaled Z-axis
Row 3: [     tx,      ty, tz,  1 ]   ← translation
```

## Interfaces

### Free Function

| Function | Signature | Description |
|----------|-----------|-------------|
| BuildWorldMatrix | `void BuildWorldMatrix(const Transform& transform, float outMatrix[4][4])` | Builds SRT world matrix from transform |

## Cross-Module Dependencies

**Depended on by**:
- `Scene/game-object` — GameObject contains a Transform member
- `Graphics/scene-rendering` — calls BuildWorldMatrix per object to compute constant
  buffer data

**Depends on**: Nothing.

## File Plan

All files already exist. No files to create or modify.

| File | Role |
|------|------|
| `Application/Math/Transform.h` | Transform struct definition |
| `Application/Math/MathUtils.h` | BuildWorldMatrix function declaration |
| `Application/Math/MathUtils.cpp` | BuildWorldMatrix function implementation |
