#include "Core/PCH.h"
#include "Graphics/GraphicsDevice.h"

namespace BA
{

using namespace Microsoft::WRL;

namespace
{

#ifdef BA_EDITOR
constexpr FLOAT kBackgroundColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};
#else
constexpr FLOAT kBackgroundColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
#endif // BA_EDITOR

} // namespace

void GraphicsDevice::Initialize(HWND window)
{
    m_window = window;

    CreateDevice();
    SetFactory();
    CreateSwapChain();
    CreateBackBufferRTV();
    CreateDepthBuffer();
    SetViewports();

    BA_LOG_INFO("GraphicsDevice initialized.");
}

void GraphicsDevice::Shutdown()
{
    m_dsv.Reset();
    m_depthTexture.Reset();
    m_backBufferRTV.Reset();
    m_swapChain.Reset();
    m_factory.Reset();
    m_deviceContext.Reset();
    m_device.Reset();

    BA_LOG_INFO("GraphicsDevice shutdown.");
}

void GraphicsDevice::BeginFrame()
{
    m_deviceContext->ClearRenderTargetView(m_backBufferRTV.Get(), kBackgroundColor);
    m_deviceContext->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    m_deviceContext->OMSetRenderTargets(1, m_backBufferRTV.GetAddressOf(), m_dsv.Get());

#ifndef BA_EDITOR
    SetViewports();
#endif // BA_EDITOR
}

void GraphicsDevice::EndFrame()
{
    BA_CRASH_IF_FAILED(m_swapChain->Present(1, 0));
}

void GraphicsDevice::Resize(UINT width, UINT height)
{
    // Client area height can legitimately collapse to 0 when the user shrinks
    // the window below the title bar + borders. Skip the resize in that case
    // so the swap chain keeps its last valid dimensions.
    if (width == 0 || height == 0)
    {
        return;
    }

    m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    m_backBufferRTV.Reset();
    m_dsv.Reset();
    m_depthTexture.Reset();
    BA_CRASH_IF_FAILED(m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0));
    CreateBackBufferRTV();
    CreateDepthBuffer();
    SetViewports();
}

#ifdef BA_EDITOR
void GraphicsDevice::RestoreBackBuffer()
{
    m_deviceContext->OMSetRenderTargets(1, m_backBufferRTV.GetAddressOf(), m_dsv.Get());
    SetViewports();
}
#endif // BA_EDITOR

ID3D11Device* GraphicsDevice::GetDevice() const
{
    return m_device.Get();
}

ID3D11DeviceContext* GraphicsDevice::GetDeviceContext() const
{
    return m_deviceContext.Get();
}

float GraphicsDevice::GetAspectRatio() const
{
    BA_ASSERT(m_swapChain.Get());

    DXGI_SWAP_CHAIN_DESC1 desc = {};
    m_swapChain->GetDesc1(&desc);
    BA_ASSERT(desc.Width > 0 && desc.Height > 0);

    return static_cast<float>(desc.Width) / static_cast<float>(desc.Height);
}

ComPtr<ID3D11Buffer> GraphicsDevice::CreateVertexBuffer(const void* data, UINT byteWidth)
{
    BA_ASSERT(m_device.Get());
    BA_ASSERT(data);
    BA_ASSERT(byteWidth > 0);

    D3D11_BUFFER_DESC desc = {
        .ByteWidth = byteWidth,
        .Usage = D3D11_USAGE_IMMUTABLE,
        .BindFlags = D3D11_BIND_VERTEX_BUFFER,
    };

    D3D11_SUBRESOURCE_DATA initData = {
        .pSysMem = data,
    };

    ComPtr<ID3D11Buffer> buffer;
    BA_CRASH_IF_FAILED(m_device->CreateBuffer(&desc, &initData, buffer.GetAddressOf()));

    return buffer;
}

ComPtr<ID3D11Buffer> GraphicsDevice::CreateIndexBuffer(const void* data, UINT byteWidth)
{
    BA_ASSERT(m_device.Get());
    BA_ASSERT(data);
    BA_ASSERT(byteWidth > 0);

    D3D11_BUFFER_DESC desc = {
        .ByteWidth = byteWidth,
        .Usage = D3D11_USAGE_IMMUTABLE,
        .BindFlags = D3D11_BIND_INDEX_BUFFER,
    };

    D3D11_SUBRESOURCE_DATA initData = {
        .pSysMem = data,
    };

    ComPtr<ID3D11Buffer> buffer;
    BA_CRASH_IF_FAILED(m_device->CreateBuffer(&desc, &initData, buffer.GetAddressOf()));

    return buffer;
}

ComPtr<ID3D11ShaderResourceView> GraphicsDevice::CreateTextureRgba8SRV(const void* pixels, UINT width, UINT height)
{
    BA_ASSERT(m_device.Get());
    BA_ASSERT(pixels);
    BA_ASSERT(width > 0 && height > 0);

    D3D11_TEXTURE2D_DESC texDesc = {
        .Width = width,
        .Height = height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc = {.Count = 1, .Quality = 0},
        .Usage = D3D11_USAGE_IMMUTABLE,
        .BindFlags = D3D11_BIND_SHADER_RESOURCE,
    };

    constexpr UINT kBytesPerPixel = 4;
    D3D11_SUBRESOURCE_DATA initData = {
        .pSysMem = pixels,
        .SysMemPitch = width * kBytesPerPixel,
    };

    ComPtr<ID3D11Texture2D> texture;
    BA_CRASH_IF_FAILED(m_device->CreateTexture2D(&texDesc, &initData, texture.GetAddressOf()));

    ComPtr<ID3D11ShaderResourceView> srv;
    BA_CRASH_IF_FAILED(m_device->CreateShaderResourceView(texture.Get(), nullptr, srv.GetAddressOf()));

    return srv;
}

ComPtr<ID3D11SamplerState> GraphicsDevice::CreateLinearWrapSampler()
{
    BA_ASSERT(m_device.Get());

    D3D11_SAMPLER_DESC desc = {
        .Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
        .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
        .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
        .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
        .MipLODBias = 0.0f,
        .MaxAnisotropy = 1,
        .ComparisonFunc = D3D11_COMPARISON_NEVER,
        .MinLOD = 0.0f,
        .MaxLOD = D3D11_FLOAT32_MAX,
    };

    ComPtr<ID3D11SamplerState> sampler;
    BA_CRASH_IF_FAILED(m_device->CreateSamplerState(&desc, sampler.GetAddressOf()));

    return sampler;
}

void GraphicsDevice::CreateDevice()
{
    D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_0};

#ifdef _DEBUG
    UINT flags = D3D11_CREATE_DEVICE_DEBUG;
#else
    UINT flags = 0;
#endif // _DEBUG

    BA_CRASH_IF_FAILED(D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        m_device.GetAddressOf(),
        nullptr,
        m_deviceContext.GetAddressOf()
    ));
}

void GraphicsDevice::SetFactory()
{
    BA_ASSERT(m_device.Get());

    ComPtr<IDXGIDevice> device;
    BA_CRASH_IF_FAILED(m_device.As(&device));

    ComPtr<IDXGIAdapter> adapter;
    BA_CRASH_IF_FAILED(device->GetAdapter(adapter.GetAddressOf()));

    BA_CRASH_IF_FAILED(adapter->GetParent(IID_PPV_ARGS(m_factory.GetAddressOf())));
}

void GraphicsDevice::CreateSwapChain()
{
    BA_ASSERT(m_factory.Get());

    DXGI_SWAP_CHAIN_DESC1 desc = {
        .Width = 0,
        .Height = 0,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc = { .Count = 1, .Quality = 0 },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 2,
        .Scaling = DXGI_SCALING_STRETCH,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
        .Flags = 0
    };

    BA_CRASH_IF_FAILED(m_factory->CreateSwapChainForHwnd(
        m_device.Get(),
        m_window,
        &desc,
        nullptr,
        nullptr,
        m_swapChain.GetAddressOf()
    ));
}

void GraphicsDevice::CreateBackBufferRTV()
{
    BA_ASSERT(m_swapChain.Get());

    ComPtr<ID3D11Texture2D> backBuffer;
    BA_CRASH_IF_FAILED(m_swapChain->GetBuffer(
        0,
        IID_PPV_ARGS(backBuffer.GetAddressOf())
    ));

    BA_CRASH_IF_FAILED(m_device->CreateRenderTargetView(
        backBuffer.Get(),
        nullptr,
        m_backBufferRTV.GetAddressOf()
    ));
}

void GraphicsDevice::CreateDepthBuffer()
{
    BA_ASSERT(m_swapChain.Get());

    DXGI_SWAP_CHAIN_DESC1 desc = {};
    m_swapChain->GetDesc1(&desc);

    D3D11_TEXTURE2D_DESC depthDesc = {
        .Width = desc.Width,
        .Height = desc.Height,
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
}

void GraphicsDevice::SetViewports()
{
    BA_ASSERT(m_swapChain.Get());

    DXGI_SWAP_CHAIN_DESC1 desc = {};
    m_swapChain->GetDesc1(&desc);

    D3D11_VIEWPORT viewport = {
        .TopLeftX = 0,
        .TopLeftY = 0,
        .Width = static_cast<FLOAT>(desc.Width),
        .Height = static_cast<FLOAT>(desc.Height),
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f
    };

    m_deviceContext->RSSetViewports(1, &viewport);
}

std::unique_ptr<GraphicsDevice> g_graphicsDevice;

} // namespace BA
