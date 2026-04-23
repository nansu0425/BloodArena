#include "Core/PCH.h"
#include "Graphics/SceneRenderer.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/Vertex.h"
#include "Graphics/ModelLibrary.h"
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
    float baseColorFactor[4];
};

namespace
{

void DrawNode(
    ID3D11DeviceContext* ctx,
    const Model& model,
    int nodeIndex,
    const Matrix& parentAccumulated,
    const Matrix& objectWorld,
    const Matrix& viewMatrix,
    const Matrix& projectionMatrix,
    ID3D11Buffer* constantBuffer)
{
    // Row-vector convention (see ModelLoader.cpp ComputeNodeLocalTransform): v' = v * (local * parent).
    const Node& node = model.nodes[nodeIndex];
    Matrix accumulated = node.localTransform * parentAccumulated;

    if (node.meshIndex >= 0)
    {
        Matrix finalWorld = accumulated * objectWorld;
        const Mesh& mesh = model.meshes[node.meshIndex];

        for (const Primitive& prim : mesh.primitives)
        {
            const Texture* texture = nullptr;
            constexpr float kIdentityFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
            const float* baseColorFactor = kIdentityFactor;
            if (prim.materialIndex >= 0 && prim.materialIndex < static_cast<int>(model.materials.size()))
            {
                const Material& material = model.materials[prim.materialIndex];
                texture = material.diffuseTextureName.empty()
                    ? g_textureLibrary->GetDefaultTexture()
                    : g_textureLibrary->FindTexture(material.diffuseTextureName);
                baseColorFactor = material.baseColorFactor;
            }
            if (!texture)
            {
                texture = g_textureLibrary->GetDefaultTexture();
            }
            BA_ASSERT(texture);

            UINT stride = sizeof(Vertex);
            UINT offset = 0;
            ctx->IASetVertexBuffers(0, 1, prim.vertexBuffer.GetAddressOf(), &stride, &offset);
            DXGI_FORMAT indexFormat = prim.isIndex32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
            ctx->IASetIndexBuffer(prim.indexBuffer.Get(), indexFormat, 0);
            ctx->PSSetShaderResources(0, 1, texture->srv.GetAddressOf());

            D3D11_MAPPED_SUBRESOURCE mapped = {};
            BA_CRASH_IF_FAILED(ctx->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));

            ObjectConstants* constants = static_cast<ObjectConstants*>(mapped.pData);
            constants->worldMatrix = finalWorld;
            constants->viewMatrix = viewMatrix;
            constants->projectionMatrix = projectionMatrix;
            constants->baseColorFactor[0] = baseColorFactor[0];
            constants->baseColorFactor[1] = baseColorFactor[1];
            constants->baseColorFactor[2] = baseColorFactor[2];
            constants->baseColorFactor[3] = baseColorFactor[3];

            ctx->Unmap(constantBuffer, 0);
            ctx->DrawIndexed(prim.indexCount, 0, 0);
        }
    }

    for (int childIndex : node.childIndices)
    {
        DrawNode(ctx, model, childIndex, accumulated, objectWorld,
                 viewMatrix, projectionMatrix, constantBuffer);
    }
}

} // namespace

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
    m_deviceContext->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    m_deviceContext->PSSetSamplers(0, 1, m_linearWrapSampler.GetAddressOf());

    Matrix viewMatrix = g_camera->GetViewMatrix();
    Matrix projectionMatrix = g_camera->GetProjectionMatrix(aspect);

    for (const GameObject& gameObject : g_scene->GetGameObjects())
    {
        if (!gameObject.modelComponent)
        {
            continue;
        }
        const Model* model = g_modelLibrary->FindModel(gameObject.modelComponent->modelName);
        BA_ASSERT(model);

        Matrix objectWorld = BuildWorld(gameObject.transform);
        for (int rootIndex : model->rootNodeIndices)
        {
            DrawNode(m_deviceContext, *model, rootIndex,
                     Matrix::Identity, objectWorld,
                     viewMatrix, projectionMatrix,
                     m_constantBuffer.Get());
        }
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
