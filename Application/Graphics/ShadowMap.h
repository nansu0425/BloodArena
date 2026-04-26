#pragma once

namespace BA
{

inline constexpr UINT kDefaultShadowMapResolution = 2048;

class ShadowMap
{
public:
    void Initialize(UINT resolution);
    void Shutdown();

    void BeginPass(ID3D11DeviceContext* ctx);

    ID3D11ShaderResourceView* GetSRV() const;

private:
    UINT m_resolution = 0;

    Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_texture;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>   m_dsv;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
};

} // namespace BA
