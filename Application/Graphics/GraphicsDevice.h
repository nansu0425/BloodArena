#pragma once

namespace BA
{

class GraphicsDevice
{
public:
    void Initialize(HWND window);
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void Resize(UINT width, UINT height);

    void RestoreBackBuffer();

    ID3D11Device* GetDevice() const;
    ID3D11DeviceContext* GetDeviceContext() const;

private:
    void CreateDevice();
    void SetFactory();
    void CreateSwapChain();
    void CreateBackBufferRTV();
    void SetViewports();

private:
    HWND m_window = nullptr;

    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
    Microsoft::WRL::ComPtr<IDXGIFactory3> m_factory;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_backBufferRTV;
};

extern std::unique_ptr<GraphicsDevice> g_graphicsDevice;

} // namespace BA
