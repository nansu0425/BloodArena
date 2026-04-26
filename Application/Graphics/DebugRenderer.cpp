#include "Core/PCH.h"
#if defined(BA_EDITOR)

#include "Graphics/DebugRenderer.h"
#include "Graphics/GraphicsDevice.h"
#include "Core/PathUtils.h"

namespace BA
{

using namespace Microsoft::WRL;

namespace
{

struct DebugVertex
{
    Vector3 position;
};

struct DebugFrustumConstants
{
    Matrix  frustumInverseViewProj;
    Matrix  cameraView;
    Matrix  cameraProjection;
    Vector4 color;
};

constexpr int kCubeVertexCount    = 8;
constexpr int kCubeLineIndexCount = 24;
constexpr int kCubeFaceIndexCount = 36;

// D3D LH NDC z-range is [0, 1]; near face at z=0, far face at z=1.
const DebugVertex kCubeVertices[kCubeVertexCount] = {
    {{-1.0f, -1.0f, 0.0f}},
    {{+1.0f, -1.0f, 0.0f}},
    {{+1.0f, +1.0f, 0.0f}},
    {{-1.0f, +1.0f, 0.0f}},
    {{-1.0f, -1.0f, 1.0f}},
    {{+1.0f, -1.0f, 1.0f}},
    {{+1.0f, +1.0f, 1.0f}},
    {{-1.0f, +1.0f, 1.0f}},
};

const uint16_t kCubeLineIndices[kCubeLineIndexCount] = {
    0,1, 1,2, 2,3, 3,0,
    4,5, 5,6, 6,7, 7,4,
    0,4, 1,5, 2,6, 3,7,
};

const uint16_t kCubeFaceIndices[kCubeFaceIndexCount] = {
    0,1,2, 0,2,3,
    4,6,5, 4,7,6,
    0,3,7, 0,7,4,
    1,5,6, 1,6,2,
    0,4,5, 0,5,1,
    3,2,6, 3,6,7,
};

} // namespace

void DebugRenderer::Initialize()
{
    BA_ASSERT(g_graphicsDevice);

    m_device = g_graphicsDevice->GetDevice();
    m_deviceContext = g_graphicsDevice->GetDeviceContext();

    CompileShaders();
    CreateCubeBuffers();
    CreateConstantBuffer();
    CreateRasterizerStates();
    CreateDepthState();
    CreateBlendStates();

    BA_LOG_INFO("DebugRenderer initialized.");
}

void DebugRenderer::Shutdown()
{
    m_opaqueBlendState.Reset();
    m_alphaBlendState.Reset();
    m_depthState.Reset();
    m_rasterizerState.Reset();
    m_constantBuffer.Reset();
    m_cubeLineIndexBuffer.Reset();
    m_cubeFaceIndexBuffer.Reset();
    m_cubeVertexBuffer.Reset();
    m_inputLayout.Reset();
    m_pixelShader.Reset();
    m_vertexShader.Reset();
    m_deviceContext = nullptr;
    m_device = nullptr;

    BA_LOG_INFO("DebugRenderer shutdown.");
}

void DebugRenderer::DrawFrustum(
    const Matrix&  frustumViewProjection,
    const Matrix&  cameraView,
    const Matrix&  cameraProjection,
    const Vector3& color,
    ID3D11RenderTargetView* rtv,
    ID3D11DepthStencilView* dsv,
    UINT viewportWidth,
    UINT viewportHeight)
{
    D3D11_VIEWPORT vp = {
        .TopLeftX = 0.0f,
        .TopLeftY = 0.0f,
        .Width    = static_cast<float>(viewportWidth),
        .Height   = static_cast<float>(viewportHeight),
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f,
    };
    m_deviceContext->RSSetViewports(1, &vp);
    m_deviceContext->OMSetRenderTargets(1, &rtv, dsv);

    UINT stride = sizeof(DebugVertex);
    UINT offset = 0;
    m_deviceContext->IASetInputLayout(m_inputLayout.Get());
    m_deviceContext->IASetVertexBuffers(0, 1, m_cubeVertexBuffer.GetAddressOf(), &stride, &offset);
    m_deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    m_deviceContext->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    m_deviceContext->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    m_deviceContext->RSSetState(m_rasterizerState.Get());
    m_deviceContext->OMSetDepthStencilState(m_depthState.Get(), 0);

    DebugFrustumConstants cb = {};
    cb.frustumInverseViewProj = frustumViewProjection.Invert();
    cb.cameraView             = cameraView;
    cb.cameraProjection       = cameraProjection;

    const float blendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    cb.color = Vector4(color.x, color.y, color.z, kDebugFrustumFaceAlpha);
    g_graphicsDevice->UpdateConstantBuffer(m_constantBuffer.Get(), cb);
    m_deviceContext->IASetIndexBuffer(m_cubeFaceIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_deviceContext->OMSetBlendState(m_alphaBlendState.Get(), blendFactor, 0xFFFFFFFF);
    m_deviceContext->DrawIndexed(kCubeFaceIndexCount, 0, 0);

    cb.color = Vector4(color.x, color.y, color.z, kDebugFrustumLineAlpha);
    g_graphicsDevice->UpdateConstantBuffer(m_constantBuffer.Get(), cb);
    m_deviceContext->IASetIndexBuffer(m_cubeLineIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    m_deviceContext->OMSetBlendState(m_opaqueBlendState.Get(), blendFactor, 0xFFFFFFFF);
    m_deviceContext->DrawIndexed(kCubeLineIndexCount, 0, 0);
}

void DebugRenderer::CompileShaders()
{
    BA_ASSERT(m_device);

    ComPtr<ID3DBlob> vsBlob = CompileShader(L"Assets/Shaders/DebugFrustumVertexShader.hlsl", "vs_5_0");
    ComPtr<ID3DBlob> psBlob = CompileShader(L"Assets/Shaders/DebugFrustumPixelShader.hlsl", "ps_5_0");

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

void DebugRenderer::CreateInputLayout(ID3DBlob* vsBlob)
{
    BA_ASSERT(m_device);
    BA_ASSERT(vsBlob);

    D3D11_INPUT_ELEMENT_DESC elements[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    BA_CRASH_IF_FAILED(m_device->CreateInputLayout(
        elements,
        _countof(elements),
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        m_inputLayout.GetAddressOf()
    ));
}

void DebugRenderer::CreateCubeBuffers()
{
    BA_ASSERT(g_graphicsDevice);

    m_cubeVertexBuffer    = g_graphicsDevice->CreateVertexBuffer(kCubeVertices, sizeof(kCubeVertices));
    m_cubeFaceIndexBuffer = g_graphicsDevice->CreateIndexBuffer(kCubeFaceIndices, sizeof(kCubeFaceIndices));
    m_cubeLineIndexBuffer = g_graphicsDevice->CreateIndexBuffer(kCubeLineIndices, sizeof(kCubeLineIndices));
}

void DebugRenderer::CreateConstantBuffer()
{
    BA_ASSERT(g_graphicsDevice);

    m_constantBuffer = g_graphicsDevice->CreateConstantBuffer(sizeof(DebugFrustumConstants));
}

void DebugRenderer::CreateRasterizerStates()
{
    BA_ASSERT(m_device);

    D3D11_RASTERIZER_DESC desc = {
        .FillMode              = D3D11_FILL_SOLID,
        .CullMode              = D3D11_CULL_NONE,
        .FrontCounterClockwise = FALSE,
        .DepthClipEnable       = TRUE,
    };

    BA_CRASH_IF_FAILED(m_device->CreateRasterizerState(
        &desc,
        m_rasterizerState.GetAddressOf()
    ));
}

void DebugRenderer::CreateDepthState()
{
    BA_ASSERT(m_device);

    D3D11_DEPTH_STENCIL_DESC desc = {
        .DepthEnable    = TRUE,
        .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO,
        .DepthFunc      = D3D11_COMPARISON_LESS,
        .StencilEnable  = FALSE,
    };

    BA_CRASH_IF_FAILED(m_device->CreateDepthStencilState(
        &desc,
        m_depthState.GetAddressOf()
    ));
}

void DebugRenderer::CreateBlendStates()
{
    BA_ASSERT(m_device);

    D3D11_BLEND_DESC alphaDesc = {};
    alphaDesc.RenderTarget[0].BlendEnable           = TRUE;
    alphaDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
    alphaDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
    alphaDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
    alphaDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
    alphaDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA;
    alphaDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    alphaDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    BA_CRASH_IF_FAILED(m_device->CreateBlendState(
        &alphaDesc,
        m_alphaBlendState.GetAddressOf()
    ));

    D3D11_BLEND_DESC opaqueDesc = {};
    opaqueDesc.RenderTarget[0].BlendEnable           = FALSE;
    opaqueDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    BA_CRASH_IF_FAILED(m_device->CreateBlendState(
        &opaqueDesc,
        m_opaqueBlendState.GetAddressOf()
    ));
}

ComPtr<ID3DBlob> DebugRenderer::CompileShader(const wchar_t* filePath, const char* target)
{
    std::wstring resolvedPath = ResolveAssetPath(filePath);

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
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
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

std::unique_ptr<DebugRenderer> g_debugRenderer;

} // namespace BA

#endif // BA_EDITOR
