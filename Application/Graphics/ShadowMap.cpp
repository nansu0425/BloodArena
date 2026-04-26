#include "Core/PCH.h"
#include "Graphics/ShadowMap.h"
#include "Graphics/GraphicsDevice.h"

namespace BA
{

void ShadowMap::Initialize(UINT resolution)
{
    BA_PROFILE_SCOPE("ShadowMap::Initialize");

    BA_ASSERT(resolution > 0);
    BA_ASSERT(g_graphicsDevice);

    DepthTextureResources resources = g_graphicsDevice->CreateDepthTexture(resolution, resolution);
    m_texture    = std::move(resources.texture);
    m_dsv        = std::move(resources.dsv);
    m_srv        = std::move(resources.srv);
    m_resolution = resolution;

    BA_LOG_INFO("ShadowMap initialized ({}x{}).", resolution, resolution);
}

void ShadowMap::Shutdown()
{
    m_srv.Reset();
    m_dsv.Reset();
    m_texture.Reset();
    m_resolution = 0;

    BA_LOG_INFO("ShadowMap shutdown.");
}

void ShadowMap::BeginPass(ID3D11DeviceContext* ctx)
{
    BA_ASSERT(ctx);
    BA_ASSERT(m_dsv.Get());

    ctx->OMSetRenderTargets(0, nullptr, m_dsv.Get());
    ctx->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    D3D11_VIEWPORT viewport = {
        .TopLeftX = 0.0f,
        .TopLeftY = 0.0f,
        .Width    = static_cast<FLOAT>(m_resolution),
        .Height   = static_cast<FLOAT>(m_resolution),
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f,
    };
    ctx->RSSetViewports(1, &viewport);
}

ID3D11ShaderResourceView* ShadowMap::GetSRV() const
{
    return m_srv.Get();
}

} // namespace BA
