#pragma once

namespace BA
{

class SceneRenderer
{
public:
    void Initialize();
    void Shutdown();

    void Render(float aspect);

private:
    void CompileShaders();
    Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const wchar_t* filePath, const char* target);
    void CreateInputLayout(ID3DBlob* vsBlob);
    void CreateConstantBuffer();
    void CreateRasterizerState();
    void CreateDepthStencilState();

private:
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_deviceContext = nullptr;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_linearWrapSampler;
};

extern std::unique_ptr<SceneRenderer> g_sceneRenderer;

} // namespace BA
