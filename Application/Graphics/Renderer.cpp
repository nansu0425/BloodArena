#include "Core/PCH.h"
#include "Graphics/Renderer.h"

namespace BA
{

using namespace Microsoft::WRL;

void Renderer::Initialize(HWND window)
{
    m_window = window;

    CreateDevice();
    SetFactory();
    CreateSwapChain();
    CreateBackBufferRTV();
    SetViewports();
}

void Renderer::Shutdown()
{
}

void Renderer::BeginFrame()
{
    const FLOAT clearColor[4] = {0.392f, 0.584f, 0.929f, 1.0f};

    m_deviceContext->ClearRenderTargetView(m_backBufferRTV.Get(), clearColor);
    m_deviceContext->OMSetRenderTargets(1, m_backBufferRTV.GetAddressOf(), nullptr);
}

void Renderer::EndFrame()
{
    BA_CRASH_IF_FAILED(m_swapChain->Present(1, 0));
}

void Renderer::CreateDevice()
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

void Renderer::SetFactory()
{
    BA_ASSERT(m_device.Get());

    ComPtr<IDXGIDevice> device;
    BA_CRASH_IF_FAILED(m_device.As(&device));

    ComPtr<IDXGIAdapter> adapter;
    BA_CRASH_IF_FAILED(device->GetAdapter(adapter.GetAddressOf()));

    BA_CRASH_IF_FAILED(adapter->GetParent(IID_PPV_ARGS(m_factory.GetAddressOf())));
}

void Renderer::CreateSwapChain()
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

void Renderer::CreateBackBufferRTV()
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

void Renderer::SetViewports()
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

std::unique_ptr<Renderer> g_renderer;

} // namespace BA
