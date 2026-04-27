#include "Core/PCH.h"
#include "Graphics/SceneViewport.h"
#include "Graphics/GraphicsDevice.h"
#include "Editor/EditorCamera.h"

namespace BA
{

SceneViewport::SceneViewport() = default;
SceneViewport::~SceneViewport() = default;

void SceneViewport::Initialize()
{
    BA_ASSERT(g_graphicsDevice);

    m_device = g_graphicsDevice->GetDevice();
    m_deviceContext = g_graphicsDevice->GetDeviceContext();

    m_editorCamera = std::make_unique<EditorCamera>();
    m_editorCamera->Initialize();

    BA_LOG_INFO("SceneViewport initialized.");
}

void SceneViewport::Shutdown()
{
    if (m_editorCamera)
    {
        m_editorCamera->Shutdown();
        m_editorCamera.reset();
    }

    m_depthSRV.Reset();
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
    m_depthSRV.Reset();
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

    // Typeless storage so the texture can be both a depth/stencil target and
    // a shader resource (SRV reads the depth channel for debug overlays etc.).
    D3D11_TEXTURE2D_DESC depthDesc = {
        .Width = width,
        .Height = height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_R24G8_TYPELESS,
        .SampleDesc = {.Count = 1, .Quality = 0},
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE,
    };

    BA_CRASH_IF_FAILED(m_device->CreateTexture2D(
        &depthDesc,
        nullptr,
        m_depthTexture.GetAddressOf()
    ));

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {
        .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
        .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
        .Texture2D = {.MipSlice = 0},
    };
    BA_CRASH_IF_FAILED(m_device->CreateDepthStencilView(
        m_depthTexture.Get(),
        &dsvDesc,
        m_dsv.GetAddressOf()
    ));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {
        .Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
        .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
        .Texture2D = {.MostDetailedMip = 0, .MipLevels = 1},
    };
    BA_CRASH_IF_FAILED(m_device->CreateShaderResourceView(
        m_depthTexture.Get(),
        &srvDesc,
        m_depthSRV.GetAddressOf()
    ));

    m_width = width;
    m_height = height;

    BA_ASSERT(m_editorCamera);
    m_editorCamera->SetAspect(static_cast<float>(width) / static_cast<float>(height));
}

void SceneViewport::Update(float deltaSeconds)
{
    BA_ASSERT(m_editorCamera);
    m_editorCamera->Update(deltaSeconds);
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

EditorCamera* SceneViewport::GetEditorCamera()
{
    return m_editorCamera.get();
}

const EditorCamera* SceneViewport::GetEditorCamera() const
{
    return m_editorCamera.get();
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

ID3D11ShaderResourceView* SceneViewport::GetDepthSRV() const
{
    return m_depthSRV.Get();
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
