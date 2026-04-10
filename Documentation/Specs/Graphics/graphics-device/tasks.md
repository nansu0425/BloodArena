# Graphics Device — Tasks

> **Note**: This spec documents already-implemented code. All tasks are marked [done].

## Task 1: [done] Create D3D11 device and context

**Files:** `Application/Graphics/GraphicsDevice.h`, `Application/Graphics/GraphicsDevice.cpp`

**Description:** Define a `GraphicsDevice` class in the `BA` namespace. Implement
`CreateDevice()` which calls `D3D11CreateDevice` with hardware driver, feature level
11.0, and `D3D11_CREATE_DEVICE_DEBUG` flag in debug builds. Store the device and
device context in ComPtr members. Declare `g_graphicsDevice` as
`extern std::unique_ptr<GraphicsDevice>`.

**Acceptance Criteria:**
- D3D11 device is created with hardware driver and feature level 11.0
- Debug layer is enabled in debug builds
- Device and context are accessible via ComPtr members

---

## Task 2: [done] Obtain DXGI factory from device adapter

**Depends on:** Task 1

**Files:** `Application/Graphics/GraphicsDevice.cpp`

**Description:** Implement `SetFactory()` which queries the device for its
`IDXGIDevice`, gets the adapter, then calls `GetParent` to obtain `IDXGIFactory3`.

**Acceptance Criteria:**
- Factory is obtained from the device's own adapter chain
- Factory is a valid IDXGIFactory3 pointer

---

## Task 3: [done] Create DXGI swap chain

**Depends on:** Task 2

**Files:** `Application/Graphics/GraphicsDevice.cpp`

**Description:** Implement `CreateSwapChain()` which creates a `IDXGISwapChain1` via
`CreateSwapChainForHwnd`. Configuration: R8G8B8A8_UNORM format, 2 buffers,
FLIP_DISCARD swap effect, no MSAA, auto-sized from window.

**Acceptance Criteria:**
- Swap chain is created for the application window
- Uses flip-discard model with 2 buffers
- Format is R8G8B8A8_UNORM

---

## Task 4: [done] Create back buffer render target view

**Depends on:** Task 3

**Files:** `Application/Graphics/GraphicsDevice.cpp`

**Description:** Implement `CreateBackBufferRTV()` which retrieves the back buffer
texture from the swap chain (buffer index 0) and creates a render target view from
it.

**Acceptance Criteria:**
- Render target view is created from the swap chain's back buffer
- RTV is stored in a ComPtr member

---

## Task 5: [done] Set viewport and implement BeginFrame/EndFrame

**Depends on:** Task 4

**Files:** `Application/Graphics/GraphicsDevice.cpp`

**Description:** Implement `SetViewports()` which reads the swap chain dimensions and
sets a matching D3D11 viewport. Implement `BeginFrame()` which clears the render
target to cornflower blue (0.392, 0.584, 0.929, 1.0) and binds it. Implement
`EndFrame()` which presents with vsync (SyncInterval = 1). Implement `Shutdown()`
which resets all ComPtrs in reverse creation order.

**Acceptance Criteria:**
- Viewport matches swap chain dimensions
- Each frame clears to cornflower blue
- Present succeeds with vsync enabled
- Shutdown releases all COM objects without errors
