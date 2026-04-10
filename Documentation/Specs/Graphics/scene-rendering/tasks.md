# Scene Rendering — Tasks

> **Note**: This spec documents already-implemented code. All tasks are marked [done].

## Task 1: [done] Create shared triangle mesh vertex buffer

**Files:** `Application/Graphics/SceneRenderer.h`, `Application/Graphics/SceneRenderer.cpp`

**Description:** Define a `SceneRenderer` class with Initialize/Shutdown/Render
methods. Define a file-scope `Vertex` struct with `float position[3]`. Implement
`CreateSharedMesh()` which creates an immutable vertex buffer with 3 hardcoded
triangle vertices. Store non-owning device and context pointers from
`g_graphicsDevice`. Declare `g_sceneRenderer` as `extern std::unique_ptr<SceneRenderer>`.

**Acceptance Criteria:**
- Vertex buffer is created as immutable with 3 vertices
- Device and context pointers are obtained from g_graphicsDevice
- BA_ASSERT guards device availability

---

## Task 2: [done] Compile vertex and pixel shaders from HLSL

**Depends on:** Task 1

**Files:** `Application/Graphics/SceneRenderer.cpp`, `Application/Shaders/VertexShader.hlsl`,
`Application/Shaders/PixelShader.hlsl`

**Description:** Implement `CompileShader()` which resolves the shader file path
(relative under debugger, exe-relative standalone), calls `D3DCompileFromFile` with
debug flags in debug builds, and logs error blob contents on failure. Implement
`CompileShaders()` which compiles VS (vs_5_0) and PS (ps_5_0) and creates the shader
objects. Write VertexShader.hlsl: cbuffer ObjectConstants at b0, transforms position
by row_major world matrix, passes color. Write PixelShader.hlsl: outputs input color
to SV_TARGET.

**Acceptance Criteria:**
- Shaders compile without errors in debug and release builds
- Shader path resolution works under debugger and standalone
- Shader compilation errors are logged before crash
- Vertex shader transforms position by world matrix
- Pixel shader outputs color directly

---

## Task 3: [done] Create input layout and constant buffer

**Depends on:** Task 2

**Files:** `Application/Graphics/SceneRenderer.cpp`

**Description:** Implement `CreateInputLayout()` with a single POSITION element
(R32G32B32_FLOAT). Define a file-scope `ObjectConstants` struct with
`float worldMatrix[4][4]` and `float color[4]`. Implement `CreateConstantBuffer()`
which creates a dynamic constant buffer sized for ObjectConstants.

**Acceptance Criteria:**
- Input layout has one POSITION element matching the Vertex struct
- Constant buffer is dynamic with CPU write access
- ObjectConstants matches the HLSL cbuffer layout

---

## Task 4: [done] Implement per-object render loop

**Depends on:** Tasks 1, 2, 3

**Files:** `Application/Graphics/SceneRenderer.cpp`

**Description:** Implement `Render()` which sets pipeline state (vertex buffer,
triangle list topology, input layout, shaders, constant buffer), then iterates over
all GameObjects. For each object, Map the constant buffer with WRITE_DISCARD, write
the world matrix (via BuildWorldMatrix) and color, Unmap, and Draw(3, 0).

**Acceptance Criteria:**
- Pipeline state is set before the draw loop
- Each object's world matrix is computed from its Transform
- Each object's color is written to the constant buffer
- Colored triangles appear at correct positions on screen
- Adding/removing objects updates the rendered scene
