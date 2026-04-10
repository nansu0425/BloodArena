# Graphics Device

> **Note**: This spec documents already-implemented code.

## Context

The graphics device module manages the Direct3D 11 device lifecycle. It creates and
owns the D3D11 device, device context, DXGI swap chain, and back buffer render target
view. It provides the per-frame BeginFrame/EndFrame cycle (clear and present) and
exposes the device and context for use by other rendering modules.

This is the lowest-level graphics module — all rendering code depends on it for GPU
access.

## Functional Requirements

1. **D3D11 Device Creation**: Create a D3D11 device with hardware driver and feature
   level 11.0.

2. **Debug Layer**: Enable the D3D11 debug layer (`D3D11_CREATE_DEVICE_DEBUG`) in
   debug builds.

3. **DXGI Factory**: Obtain a `IDXGIFactory3` from the device's adapter chain for
   swap chain creation.

4. **Swap Chain**: Create a `IDXGISwapChain1` for the application window with:
   - Format: `DXGI_FORMAT_R8G8B8A8_UNORM`
   - Buffer count: 2
   - Swap effect: `DXGI_SWAP_EFFECT_FLIP_DISCARD`
   - Sample count: 1 (no MSAA)

5. **Back Buffer Render Target**: Create a render target view from the swap chain's
   back buffer.

6. **Viewport**: Set the viewport to match the swap chain dimensions.

7. **BeginFrame**: Clear the render target to cornflower blue (0.392, 0.584, 0.929, 1.0)
   and bind it as the active render target.

8. **EndFrame**: Present the swap chain with vsync (SyncInterval = 1).

9. **Device Access**: Expose raw `ID3D11Device*` and `ID3D11DeviceContext*` pointers
   for other modules.

10. **Graceful Shutdown**: Release all COM objects in reverse creation order.

## Non-Functional Requirements

1. All D3D11/DXGI API failures must crash via `BA_CRASH_IF_FAILED`.
2. All COM pointers must be managed via `Microsoft::WRL::ComPtr`.
3. Assertions (`BA_ASSERT`) must guard preconditions in private setup methods.

## Out of Scope

- Window resize handling or swap chain recreation.
- Depth/stencil buffer.
- Multi-adapter or software rasterizer support.
- Configurable clear color or present interval.

## Dependencies

- **Specs**: `Core/diagnostics` (BA_ASSERT, BA_CRASH_IF_FAILED, BA_LOG_INFO)
- **Specs**: `Core/window` (provides HWND for swap chain)
- **Platform**: Direct3D 11 SDK (d3d11.lib, dxgi.lib)

## Acceptance Criteria

1. Application window shows a solid cornflower blue background.
2. No D3D11 device creation errors in debug or release builds.
3. Present succeeds each frame without errors.
4. `GetDevice()` and `GetDeviceContext()` return valid, non-null pointers after
   initialization.
