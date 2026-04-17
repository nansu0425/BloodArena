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

#ifdef BA_EDITOR
    void RestoreBackBuffer();
#endif // BA_EDITOR

    ID3D11Device* GetDevice() const;
    ID3D11DeviceContext* GetDeviceContext() const;
    float GetAspectRatio() const;

    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateVertexBuffer(const void* data, UINT byteWidth);
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateIndexBuffer(const void* data, UINT byteWidth);
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateTextureRgba8SRV(const void* pixels, UINT width, UINT height);
    Microsoft::WRL::ComPtr<ID3D11SamplerState> CreateLinearWrapSampler();

private:
    void CreateDevice();
    void SetFactory();
    void CreateSwapChain();
    void CreateBackBufferRTV();
    void CreateDepthBuffer();
    void SetViewports();

private:
    HWND m_window = nullptr;

    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
    Microsoft::WRL::ComPtr<IDXGIFactory3> m_factory;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_backBufferRTV;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthTexture;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;
};

extern std::unique_ptr<GraphicsDevice> g_graphicsDevice;

} // namespace BA
