#include "Core/PCH.h"
#include "Graphics/SceneViewport.h"
#include "Graphics/GraphicsDevice.h"

namespace BA
{

void SceneViewport::Initialize()
{
    BA_ASSERT(g_graphicsDevice);

    m_device = g_graphicsDevice->GetDevice();
    m_deviceContext = g_graphicsDevice->GetDeviceContext();

    BA_LOG_INFO("SceneViewport initialized.");
}

void SceneViewport::Shutdown()
{
    m_dsv.Reset();
    m_depthTexture.Reset();
    m_srv.Reset();
    m_rtv.Reset();
    m_texture.Reset();
    m_deviceContext = nullptr;
    m_device = nullptr;

    BA_LOG_INFO("SceneViewport shutdown.");
}

void SceneViewport::Resize(UINT width, UINT height)
{
    BA_ASSERT(width > 0 && height > 0);

    if (width == m_width && height == m_height)
    {
        return;
    }

    m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    m_srv.Reset();
    m_rtv.Reset();
    m_texture.Reset();
    m_dsv.Reset();
    m_depthTexture.Reset();

    D3D11_TEXTURE2D_DESC texDesc = {
        .Width = width,
        .Height = height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc = {.Count = 1, .Quality = 0},
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
    };

    BA_CRASH_IF_FAILED(m_device->CreateTexture2D(
        &texDesc,
        nullptr,
        m_texture.GetAddressOf()
    ));

    BA_CRASH_IF_FAILED(m_device->CreateRenderTargetView(
        m_texture.Get(),
        nullptr,
        m_rtv.GetAddressOf()
    ));

    BA_CRASH_IF_FAILED(m_device->CreateShaderResourceView(
        m_texture.Get(),
        nullptr,
        m_srv.GetAddressOf()
    ));

    D3D11_TEXTURE2D_DESC depthDesc = {
        .Width = width,
        .Height = height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
        .SampleDesc = {.Count = 1, .Quality = 0},
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_DEPTH_STENCIL,
    };

    BA_CRASH_IF_FAILED(m_device->CreateTexture2D(
        &depthDesc,
        nullptr,
        m_depthTexture.GetAddressOf()
    ));

    BA_CRASH_IF_FAILED(m_device->CreateDepthStencilView(
        m_depthTexture.Get(),
        nullptr,
        m_dsv.GetAddressOf()
    ));

    m_width = width;
    m_height = height;
}

void SceneViewport::Clear()
{
    static constexpr FLOAT kSceneClearColor[4] = {0.392f, 0.584f, 0.929f, 1.0f};

    m_deviceContext->ClearRenderTargetView(m_rtv.Get(), kSceneClearColor);
    m_deviceContext->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    m_deviceContext->OMSetRenderTargets(1, m_rtv.GetAddressOf(), m_dsv.Get());

    D3D11_VIEWPORT vp = {
        .TopLeftX = 0,
        .TopLeftY = 0,
        .Width = static_cast<FLOAT>(m_width),
        .Height = static_cast<FLOAT>(m_height),
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f,
    };
    m_deviceContext->RSSetViewports(1, &vp);
}

ID3D11RenderTargetView* SceneViewport::GetRTV() const
{
    return m_rtv.Get();
}

ID3D11DepthStencilView* SceneViewport::GetDSV() const
{
    return m_dsv.Get();
}

ID3D11ShaderResourceView* SceneViewport::GetSRV() const
{
    return m_srv.Get();
}

UINT SceneViewport::GetWidth() const
{
    return m_width;
}

UINT SceneViewport::GetHeight() const
{
    return m_height;
}

std::unique_ptr<SceneViewport> g_sceneViewport;

} // namespace BA
