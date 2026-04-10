# Graphics Device — Design

> **Note**: This spec documents already-implemented code.

## Architecture

GraphicsDevice is the lowest-level graphics module. It owns all GPU resources needed
for rendering and provides the frame lifecycle that other rendering modules plug into.

```
Lifecycle::Run()
 └── Frame loop
      ├── GraphicsDevice::BeginFrame()  — clear + bind render target
      ├── SceneRenderer::Render()       — uses Device/Context
      ├── EditorUI::Render()            — uses Device/Context
      └── GraphicsDevice::EndFrame()    — present swap chain
```

## Key Decisions

1. **Factory obtained from device adapter chain**: Instead of calling
   `CreateDXGIFactory` directly, the factory is obtained by querying the device's
   adapter via `IDXGIDevice → IDXGIAdapter → GetParent`. This guarantees the factory
   matches the adapter the device was created on.

2. **FLIP_DISCARD swap effect**: Uses the modern flip model for better performance
   and compatibility with Windows 10+. Requires 2+ buffers.

3. **Swap chain dimensions set to 0**: Width and Height are set to 0 in the swap
   chain description, which tells DXGI to automatically use the window's client area
   dimensions.

4. **Viewport derived from swap chain**: The viewport dimensions are read back from
   the swap chain description rather than hardcoded, staying consistent if the swap
   chain auto-sizes.

5. **ComPtr for all COM objects**: All D3D11/DXGI interfaces are held in
   `Microsoft::WRL::ComPtr` for automatic reference counting and exception-safe
   cleanup.

6. **Reverse-order shutdown**: COM objects are reset in reverse creation order
   (RTV → swap chain → factory → context → device) to respect dependency ordering.

## Data Structures

### GraphicsDevice (class)

```cpp
namespace BA
{
class GraphicsDevice
{
public:
    void Initialize(HWND window);
    void Shutdown();
    void BeginFrame();
    void EndFrame();
    ID3D11Device* GetDevice() const;
    ID3D11DeviceContext* GetDeviceContext() const;

private:
    void CreateDevice();
    void SetFactory();
    void CreateSwapChain();
    void CreateBackBufferRTV();
    void SetViewports();

    HWND m_window = nullptr;
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_deviceContext;
    ComPtr<IDXGIFactory3> m_factory;
    ComPtr<IDXGISwapChain1> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_backBufferRTV;
};

extern std::unique_ptr<GraphicsDevice> g_graphicsDevice;
}
```

## Interfaces

### Public API

| Method | Signature | Description |
|--------|-----------|-------------|
| Initialize | `void Initialize(HWND window)` | Creates device, factory, swap chain, RTV, viewport |
| Shutdown | `void Shutdown()` | Resets all ComPtrs in reverse order |
| BeginFrame | `void BeginFrame()` | Clears render target, binds it to output merger |
| EndFrame | `void EndFrame()` | Presents swap chain with vsync |
| GetDevice | `ID3D11Device* GetDevice() const` | Returns raw device pointer |
| GetDeviceContext | `ID3D11DeviceContext* GetDeviceContext() const` | Returns raw context pointer |

### Private Methods

| Method | Description |
|--------|-------------|
| CreateDevice | D3D11CreateDevice with hardware driver, FL 11.0, debug flag in debug builds |
| SetFactory | Obtains IDXGIFactory3 via device → adapter → GetParent chain |
| CreateSwapChain | Creates IDXGISwapChain1 via CreateSwapChainForHwnd |
| CreateBackBufferRTV | Gets back buffer texture, creates render target view |
| SetViewports | Reads swap chain dimensions, sets viewport |

## Cross-Module Dependencies

**Depended on by**:
- `Graphics/scene-rendering` — uses Device and DeviceContext for rendering
- `Editor/debug-ui` — uses Device and DeviceContext for ImGui DX11 backend
- `Core/application-lifecycle` — calls BeginFrame/EndFrame in the run loop

**Depends on**:
- `Core/diagnostics` — BA_ASSERT, BA_CRASH_IF_FAILED, BA_LOG_INFO
- `Core/window` — provides HWND for swap chain creation

## File Plan

All files already exist. No files to create or modify.

| File | Role |
|------|------|
| `Application/Graphics/GraphicsDevice.h` | GraphicsDevice class declaration |
| `Application/Graphics/GraphicsDevice.cpp` | GraphicsDevice class implementation |
