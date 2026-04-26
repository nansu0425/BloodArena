#pragma once

namespace BA
{

struct DepthTextureResources
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D>          texture;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>   dsv;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
};

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
    float GetAspectRatio() const;

    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateVertexBuffer(const void* data, UINT byteWidth);
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateIndexBuffer(const void* data, UINT byteWidth);
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateConstantBuffer(UINT byteWidth);
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateTextureRgba8SRV(const void* pixels, UINT width, UINT height);
    Microsoft::WRL::ComPtr<ID3D11SamplerState> CreateLinearWrapSampler();

    DepthTextureResources CreateDepthTexture(UINT width, UINT height);
    Microsoft::WRL::ComPtr<ID3D11SamplerState> CreateShadowComparisonSampler();

    template <typename TConstants>
    void UpdateConstantBuffer(ID3D11Buffer* buffer, const TConstants& data)
    {
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        BA_CRASH_IF_FAILED(m_deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
        memcpy(mapped.pData, &data, sizeof(TConstants));
        m_deviceContext->Unmap(buffer, 0);
    }

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
