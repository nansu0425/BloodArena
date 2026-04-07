#include "Core/PCH.h"
#include "Graphics/Renderer.h"
#include "Math/MathUtils.h"
#include "Scene/Scene.h"

namespace BA
{

using namespace Microsoft::WRL;

struct Vertex
{
    float position[3];
};

struct ObjectConstants
{
    float worldMatrix[4][4];
    float color[4];
};

void Renderer::Initialize(HWND window)
{
    m_window = window;

    CreateDevice();
    SetFactory();
    CreateSwapChain();
    CreateBackBufferRTV();
    SetViewports();
    CreateSharedMesh();
    CompileShaders();
    CreateConstantBuffer();

    BA_LOG_INFO("Renderer initialized.");
}

void Renderer::Shutdown()
{
    m_constantBuffer.Reset();
    m_inputLayout.Reset();
    m_pixelShader.Reset();
    m_vertexShader.Reset();
    m_vertexBuffer.Reset();
    m_backBufferRTV.Reset();
    m_swapChain.Reset();
    m_factory.Reset();
    m_deviceContext.Reset();
    m_device.Reset();

    BA_LOG_INFO("Renderer shutdown.");
}

void Renderer::BeginFrame()
{
    const FLOAT clearColor[4] = {0.392f, 0.584f, 0.929f, 1.0f};

    m_deviceContext->ClearRenderTargetView(m_backBufferRTV.Get(), clearColor);
    m_deviceContext->OMSetRenderTargets(1, m_backBufferRTV.GetAddressOf(), nullptr);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_deviceContext->IASetInputLayout(m_inputLayout.Get());
    m_deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    m_deviceContext->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    for (const GameObject& gameObject : g_scene->GetGameObjects())
    {
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        BA_CRASH_IF_FAILED(m_deviceContext->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));

        ObjectConstants* constants = static_cast<ObjectConstants*>(mapped.pData);
        BuildWorldMatrix(gameObject.transform, constants->worldMatrix);
        constants->color[0] = gameObject.color[0];
        constants->color[1] = gameObject.color[1];
        constants->color[2] = gameObject.color[2];
        constants->color[3] = gameObject.color[3];

        m_deviceContext->Unmap(m_constantBuffer.Get(), 0);
        m_deviceContext->Draw(3, 0);
    }
}

void Renderer::EndFrame()
{
    BA_CRASH_IF_FAILED(m_swapChain->Present(1, 0));
}

void Renderer::CreateSharedMesh()
{
    BA_ASSERT(m_device.Get());

    Vertex vertices[] =
    {
        {{ 0.0f,   0.06f, 0.0f}},
        {{ 0.06f, -0.04f, 0.0f}},
        {{-0.06f, -0.04f, 0.0f}},
    };

    D3D11_BUFFER_DESC bufferDesc = {
        .ByteWidth = sizeof(vertices),
        .Usage = D3D11_USAGE_IMMUTABLE,
        .BindFlags = D3D11_BIND_VERTEX_BUFFER,
    };

    D3D11_SUBRESOURCE_DATA initData = {
        .pSysMem = vertices,
    };

    BA_CRASH_IF_FAILED(m_device->CreateBuffer(
        &bufferDesc,
        &initData,
        m_vertexBuffer.GetAddressOf()
    ));
}

void Renderer::CreateConstantBuffer()
{
    BA_ASSERT(m_device.Get());

    D3D11_BUFFER_DESC bufferDesc = {
        .ByteWidth = sizeof(ObjectConstants),
        .Usage = D3D11_USAGE_DYNAMIC,
        .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
        .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    };

    BA_CRASH_IF_FAILED(m_device->CreateBuffer(
        &bufferDesc,
        nullptr,
        m_constantBuffer.GetAddressOf()
    ));
}

void Renderer::CompileShaders()
{
    BA_ASSERT(m_device.Get());

    ComPtr<ID3DBlob> vsBlob = CompileShader(L"Shaders/VertexShader.hlsl", "vs_5_0");
    ComPtr<ID3DBlob> psBlob = CompileShader(L"Shaders/PixelShader.hlsl", "ps_5_0");

    BA_CRASH_IF_FAILED(m_device->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        m_vertexShader.GetAddressOf()
    ));

    BA_CRASH_IF_FAILED(m_device->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr,
        m_pixelShader.GetAddressOf()
    ));

    CreateInputLayout(vsBlob.Get());
}

ComPtr<ID3DBlob> Renderer::CompileShader(const wchar_t* filePath, const char* target)
{
    // Debugger attached: use relative path (CWD = ProjectDir, reads source shaders directly)
    // Standalone exe: resolve from exe directory (reads deployed shader copies)
    std::wstring resolvedPath;

    if (IsDebuggerPresent())
    {
        resolvedPath = filePath;
    }
    else
    {
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);

        resolvedPath = std::wstring(exePath).substr(0, std::wstring(exePath).rfind(L'\\') + 1);
        resolvedPath += filePath;
    }

#ifdef _DEBUG
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif // _DEBUG

    ComPtr<ID3DBlob> blob;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DCompileFromFile(
        resolvedPath.c_str(),
        nullptr,
        nullptr,
        "main",
        target,
        compileFlags,
        0,
        blob.GetAddressOf(),
        errorBlob.GetAddressOf()
    );

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            BA_CRASH_LOG(static_cast<const char*>(errorBlob->GetBufferPointer()));
        }
        else
        {
            BA_CRASH_IF_FAILED(hr);
        }
    }

    return blob;
}

void Renderer::CreateInputLayout(ID3DBlob* vsBlob)
{
    BA_ASSERT(m_device.Get());
    BA_ASSERT(vsBlob);

    D3D11_INPUT_ELEMENT_DESC inputElements[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    BA_CRASH_IF_FAILED(m_device->CreateInputLayout(
        inputElements,
        _countof(inputElements),
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        m_inputLayout.GetAddressOf()
    ));
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
