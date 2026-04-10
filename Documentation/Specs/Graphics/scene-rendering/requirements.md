# Scene Rendering

> **Note**: This spec documents already-implemented code.

## Context

The scene rendering module implements the rendering pipeline that draws game objects
on screen. It manages the vertex buffer, shaders, input layout, and constant buffer,
and iterates over all game objects each frame to render them with their individual
world transforms and colors.

This module bridges the scene data (GameObjects with transforms and colors) and the
graphics device (D3D11 rendering API).

## Functional Requirements

1. **Shared Triangle Mesh**: A hardcoded triangle mesh (3 vertices) must serve as the
   shape for all game objects. The triangle vertices are:
   - (0.0, 0.06, 0.0), (0.06, -0.04, 0.0), (-0.06, -0.04, 0.0)

2. **Per-Object Rendering**: Each game object in the scene must be rendered
   individually with its own world matrix and color.

3. **World Matrix**: Each object's world matrix must be computed from its Transform
   via `BuildWorldMatrix` and uploaded to a constant buffer.

4. **Object Color**: Each object's RGBA color must be uploaded to the constant buffer
   alongside the world matrix.

5. **Vertex Shader**: The vertex shader must:
   - Accept a `float3` POSITION input
   - Transform it by a `row_major float4x4` world matrix from constant buffer b0
   - Pass the object color through to the pixel shader

6. **Pixel Shader**: The pixel shader must output the interpolated color directly to
   `SV_TARGET`.

7. **Runtime Shader Compilation**: Shaders must be compiled at runtime from HLSL
   source files using `D3DCompileFromFile`.

8. **Shader Path Resolution**: Shader file paths must resolve correctly in both
   scenarios:
   - Under debugger: relative path from project directory
   - Standalone exe: relative path from exe directory

9. **Debug Shader Flags**: In debug builds, shaders must be compiled with
   `D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION`.

10. **Shader Error Reporting**: If shader compilation fails, the error message from
    the error blob must be logged before crashing.

## Non-Functional Requirements

1. The vertex buffer must be immutable (`D3D11_USAGE_IMMUTABLE`).
2. The constant buffer must be dynamic (`D3D11_USAGE_DYNAMIC`) with CPU write access
   for per-object updates via Map/Unmap.
3. All D3D11 API failures must crash via `BA_CRASH_IF_FAILED`.
4. All COM pointers must be managed via `Microsoft::WRL::ComPtr`.

## Out of Scope

- Index buffers or complex mesh geometry.
- Textures or materials.
- Instanced rendering.
- Depth testing or blending.
- Pre-compiled shader bytecode (CSO files).

## Dependencies

- **Specs**: `Graphics/graphics-device` (D3D11 device and context)
- **Specs**: `Math/transform` (BuildWorldMatrix)
- **Specs**: `Scene/game-object` (iterating GameObjects)
- **Specs**: `Core/diagnostics` (BA_ASSERT, BA_CRASH_IF_FAILED, BA_CRASH_LOG)

## Acceptance Criteria

1. Colored triangles appear at the correct positions for each game object.
2. Each object displays its assigned palette color.
3. Adding or removing objects updates the rendered scene immediately.
4. Shaders compile successfully in both debug and release builds.
5. Running the exe standalone (outside debugger) finds and compiles shaders correctly.
