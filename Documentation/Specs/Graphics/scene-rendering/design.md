# Scene Rendering — Design

> **Note**: This spec documents already-implemented code.

## Architecture

SceneRenderer plugs into the frame loop between `BeginFrame` and `EndFrame`. It sets
up the pipeline state once per frame, then iterates over all GameObjects, updating the
constant buffer for each and issuing a draw call.

```
GraphicsDevice::BeginFrame()
 └── SceneRenderer::Render()
      ├── Set pipeline state (vertex buffer, topology, shaders, input layout, cbuffer)
      └── For each GameObject:
           ├── Map constant buffer
           ├── Write world matrix + color
           ├── Unmap
           └── Draw(3, 0)
```

## Key Decisions

1. **One draw call per object**: Each object gets its own Map/Draw cycle. This is
   simple and correct for a small number of objects. Instanced rendering would be
   more efficient at scale but is out of scope.

2. **Dynamic constant buffer with Map/Unmap**: Using `D3D11_MAP_WRITE_DISCARD`
   provides efficient per-frame updates. The entire buffer is replaced each time,
   avoiding partial update complexity.

3. **Runtime shader compilation**: Shaders are compiled from HLSL source at startup
   rather than pre-compiled to CSO. This simplifies the build pipeline and allows
   shader iteration without a separate compilation step.

4. **Dual path resolution**: `IsDebuggerPresent()` determines whether to use relative
   paths (debugger, CWD = project dir) or exe-relative paths (standalone). This
   avoids build-system complexity for shader deployment.

5. **Error blob logging on shader failure**: When `D3DCompileFromFile` fails, the
   error blob contains the compiler diagnostic. This is logged via `BA_CRASH_LOG`
   before termination, making shader errors diagnosable.

6. **Non-owning device/context pointers**: SceneRenderer stores raw pointers to the
   device and context obtained from `g_graphicsDevice` at initialization. These are
   non-owning — GraphicsDevice owns the COM objects.

7. **Pipeline state per Vertex struct**: The `Vertex` struct contains only
   `float position[3]`, and the input layout has a single POSITION element. This
   is the minimal layout needed for the current rendering requirements.

## Data Structures

### Vertex (struct, file-scope)

```cpp
struct Vertex
{
    float position[3];
};
```

### ObjectConstants (struct, file-scope)

```cpp
struct ObjectConstants
{
    float worldMatrix[4][4];
    float color[4];
};
```

### SceneRenderer (class)

```cpp
namespace BA
{
class SceneRenderer
{
public:
    void Initialize();
    void Shutdown();
    void Render();

private:
    void CreateSharedMesh();
    void CompileShaders();
    ComPtr<ID3DBlob> CompileShader(const wchar_t* filePath, const char* target);
    void CreateInputLayout(ID3DBlob* vsBlob);
    void CreateConstantBuffer();

    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_deviceContext = nullptr;
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11Buffer> m_constantBuffer;
};

extern std::unique_ptr<SceneRenderer> g_sceneRenderer;
}
```

### HLSL Constant Buffer (register b0)

```hlsl
cbuffer ObjectConstants : register(b0)
{
    row_major float4x4 worldMatrix;
    float4 objectColor;
};
```

## Interfaces

### Public API

| Method | Signature | Description |
|--------|-----------|-------------|
| Initialize | `void Initialize()` | Gets device/context, creates mesh, compiles shaders, creates cbuffer |
| Shutdown | `void Shutdown()` | Resets all ComPtrs, nulls device/context pointers |
| Render | `void Render()` | Sets pipeline state, renders all GameObjects |

### Private Methods

| Method | Description |
|--------|-------------|
| CreateSharedMesh | Creates immutable vertex buffer with 3-vertex triangle |
| CompileShaders | Compiles VS and PS from HLSL, creates shader objects and input layout |
| CompileShader | Compiles a single HLSL file with path resolution and error reporting |
| CreateInputLayout | Creates input layout with single POSITION element |
| CreateConstantBuffer | Creates dynamic constant buffer for ObjectConstants |

### Shader Files

| File | Target | Description |
|------|--------|-------------|
| `Shaders/VertexShader.hlsl` | vs_5_0 | Transforms position by world matrix, passes color |
| `Shaders/PixelShader.hlsl` | ps_5_0 | Outputs interpolated color to render target |

## Cross-Module Dependencies

**Depended on by**:
- `Core/application-lifecycle` — calls Render() in the frame loop

**Depends on**:
- `Graphics/graphics-device` — D3D11 device and context
- `Math/transform` — BuildWorldMatrix function
- `Scene/game-object` — iterates g_scene->GetGameObjects()
- `Core/diagnostics` — BA_ASSERT, BA_CRASH_IF_FAILED, BA_CRASH_LOG

## File Plan

All files already exist. No files to create or modify.

| File | Role |
|------|------|
| `Application/Graphics/SceneRenderer.h` | SceneRenderer class declaration |
| `Application/Graphics/SceneRenderer.cpp` | SceneRenderer implementation |
| `Application/Shaders/VertexShader.hlsl` | Vertex shader (world transform + color pass) |
| `Application/Shaders/PixelShader.hlsl` | Pixel shader (color output) |
