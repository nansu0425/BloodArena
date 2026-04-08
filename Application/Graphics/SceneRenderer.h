#pragma once

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
    Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const wchar_t* filePath, const char* target);
    void CreateInputLayout(ID3DBlob* vsBlob);
    void CreateConstantBuffer();

private:
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_deviceContext = nullptr;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
};

extern std::unique_ptr<SceneRenderer> g_sceneRenderer;

} // namespace BA
