#pragma once

namespace BA
{

class Renderer
{
public:
    void Initialize(HWND window);
    void Shutdown();

    void BeginFrame();
    void EndFrame();

private:
    void CreateDevice();
    void SetFactory();
    void CreateSwapChain();
    void CreateBackBufferRTV();
    void SetViewports();
    void CreateTriangle();
    void CreateVertexBuffer();
    void CompileShaders();
    Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const wchar_t* filePath, const char* target);
    void CreateInputLayout(ID3DBlob* vsBlob);

private:
    HWND m_window = nullptr;

    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
    Microsoft::WRL::ComPtr<IDXGIFactory3> m_factory;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_backBufferRTV;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
};

extern std::unique_ptr<Renderer> g_renderer;

} // namespace BA
