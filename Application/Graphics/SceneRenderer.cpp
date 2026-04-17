#include "Core/PCH.h"
#include "Graphics/SceneRenderer.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/Vertex.h"
#include "Graphics/MeshLibrary.h"
#include "Graphics/TextureLibrary.h"
#include "Core/PathUtils.h"
#include "Math/MathUtils.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"

namespace BA
{

using namespace Microsoft::WRL;

struct ObjectConstants
{
    Matrix worldMatrix;
    Matrix viewMatrix;
    Matrix projectionMatrix;
    float color[4];
};

void SceneRenderer::Initialize()
{
    BA_ASSERT(g_graphicsDevice);

    m_device = g_graphicsDevice->GetDevice();
    m_deviceContext = g_graphicsDevice->GetDeviceContext();

    CompileShaders();
    CreateConstantBuffer();
    CreateRasterizerState();
    CreateDepthStencilState();

    m_linearWrapSampler = g_graphicsDevice->CreateLinearWrapSampler();

    BA_LOG_INFO("SceneRenderer initialized.");
}

void SceneRenderer::Shutdown()
{
    m_linearWrapSampler.Reset();
    m_depthStencilState.Reset();
    m_rasterizerState.Reset();
    m_constantBuffer.Reset();
    m_inputLayout.Reset();
    m_pixelShader.Reset();
    m_vertexShader.Reset();
    m_deviceContext = nullptr;
    m_device = nullptr;

    BA_LOG_INFO("SceneRenderer shutdown.");
}

void SceneRenderer::Render(float aspect)
{
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_deviceContext->IASetInputLayout(m_inputLayout.Get());
    m_deviceContext->RSSetState(m_rasterizerState.Get());
    m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
    m_deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    m_deviceContext->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    m_deviceContext->PSSetSamplers(0, 1, m_linearWrapSampler.GetAddressOf());

    Matrix viewMatrix = g_camera->GetViewMatrix();
    Matrix projectionMatrix = g_camera->GetProjectionMatrix(aspect);

    for (const GameObject& gameObject : g_scene->GetGameObjects())
    {
        const Mesh* mesh = g_meshLibrary->FindMesh(gameObject.meshName);
        if (!mesh)
        {
            mesh = g_meshLibrary->GetDefaultMesh();
        }
        BA_ASSERT(mesh);

        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        m_deviceContext->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(), &stride, &offset);
        DXGI_FORMAT indexFormat = mesh->isIndex32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
        m_deviceContext->IASetIndexBuffer(mesh->indexBuffer.Get(), indexFormat, 0);

        const Texture* texture = mesh->textureName.empty()
            ? g_textureLibrary->GetDefaultTexture()
            : g_textureLibrary->FindTexture(mesh->textureName);
        if (!texture)
        {
            texture = g_textureLibrary->GetDefaultTexture();
        }
        BA_ASSERT(texture);
        m_deviceContext->PSSetShaderResources(0, 1, texture->srv.GetAddressOf());

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        BA_CRASH_IF_FAILED(m_deviceContext->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));

        ObjectConstants* constants = static_cast<ObjectConstants*>(mapped.pData);
        constants->worldMatrix = BuildWorld(gameObject.transform);
        constants->viewMatrix = viewMatrix;
        constants->projectionMatrix = projectionMatrix;
        constants->color[0] = gameObject.color[0];
        constants->color[1] = gameObject.color[1];
        constants->color[2] = gameObject.color[2];
        constants->color[3] = gameObject.color[3];

        m_deviceContext->Unmap(m_constantBuffer.Get(), 0);
        m_deviceContext->DrawIndexed(mesh->indexCount, 0, 0);
    }
}

void SceneRenderer::CreateConstantBuffer()
{
    BA_ASSERT(m_device);

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

void SceneRenderer::CompileShaders()
{
    BA_ASSERT(m_device);

    ComPtr<ID3DBlob> vsBlob = CompileShader(L"Assets/Shaders/VertexShader.hlsl", "vs_5_0");
    ComPtr<ID3DBlob> psBlob = CompileShader(L"Assets/Shaders/PixelShader.hlsl", "ps_5_0");

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

ComPtr<ID3DBlob> SceneRenderer::CompileShader(const wchar_t* filePath, const char* target)
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

void SceneRenderer::CreateInputLayout(ID3DBlob* vsBlob)
{
    BA_ASSERT(m_device);
    BA_ASSERT(vsBlob);

    D3D11_INPUT_ELEMENT_DESC inputElements[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    BA_CRASH_IF_FAILED(m_device->CreateInputLayout(
        inputElements,
        _countof(inputElements),
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        m_inputLayout.GetAddressOf()
    ));
}

void SceneRenderer::CreateRasterizerState()
{
    BA_ASSERT(m_device);

    D3D11_RASTERIZER_DESC desc = {
        .FillMode = D3D11_FILL_SOLID,
        .CullMode = D3D11_CULL_BACK,
        .FrontCounterClockwise = FALSE,
        .DepthClipEnable = TRUE,
    };

    BA_CRASH_IF_FAILED(m_device->CreateRasterizerState(
        &desc,
        m_rasterizerState.GetAddressOf()
    ));
}

void SceneRenderer::CreateDepthStencilState()
{
    BA_ASSERT(m_device);

    D3D11_DEPTH_STENCIL_DESC desc = {
        .DepthEnable = TRUE,
        .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
        .DepthFunc = D3D11_COMPARISON_LESS,
        .StencilEnable = FALSE,
    };

    BA_CRASH_IF_FAILED(m_device->CreateDepthStencilState(
        &desc,
        m_depthStencilState.GetAddressOf()
    ));
}

std::unique_ptr<SceneRenderer> g_sceneRenderer;

} // namespace BA
