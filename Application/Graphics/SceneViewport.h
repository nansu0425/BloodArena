#pragma once

namespace BA
{

class SceneViewport
{
public:
    void Initialize();
    void Shutdown();

    void Resize(UINT width, UINT height);
    void Clear();

    ID3D11RenderTargetView* GetRTV() const;
    ID3D11ShaderResourceView* GetSRV() const;

    UINT GetWidth() const;
    UINT GetHeight() const;

private:
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_deviceContext = nullptr;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;

    UINT m_width = 0;
    UINT m_height = 0;
};

extern std::unique_ptr<SceneViewport> g_sceneViewport;

} // namespace BA
