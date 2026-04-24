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

namespace
{

struct ModelConstants
{
    Matrix  worldMatrix;
    Matrix  worldInverseTransposeMatrix;
    Vector4 baseColorFactor;
};

struct FrameConstants
{
    Matrix   viewMatrix;
    Matrix   projectionMatrix;
    Vector4  cameraPositionWorld;  // xyz used; w unused
    uint32_t viewMode;
    uint32_t _viewModePad[3];
};

struct DirectionalLightConstants
{
    Vector4 lightDirection;  // xyz = normalized direction of travel; w unused
    Vector4 lightColor;      // rgb = color * intensity; a unused
    Vector4 ambientColor;    // rgb used; a unused
    Vector4 specularParams;  // x = specularStrength, y = shininess; zw unused
};

// Ambient-only fallback so a scene without any directional light is still visible in Lit mode.
// Matches LightComponent's default ambientColor so Lit + no-light ≈ "add a light here" hint,
// not a black screen.
constexpr float kNoLightAmbient[3] = {0.15f, 0.15f, 0.18f};

FrameConstants BuildFrameConstants(
    const Matrix& viewMatrix,
    const Matrix& projectionMatrix,
    const Vector3& cameraPositionWorld,
    ViewMode viewMode)
{
    FrameConstants frame = {};
    frame.viewMatrix = viewMatrix;
    frame.projectionMatrix = projectionMatrix;
    frame.cameraPositionWorld = Vector4(cameraPositionWorld.x, cameraPositionWorld.y, cameraPositionWorld.z, 0.0f);
    frame.viewMode = static_cast<uint32_t>(viewMode);
    return frame;
}

DirectionalLightConstants BuildDirectionalLightConstants(const Scene& scene)
{
    Vector3 directionWorld     = kAxisForward;
    Vector3 colorPremultiplied = {0.0f, 0.0f, 0.0f};
    Vector3 ambientColor       = {kNoLightAmbient[0], kNoLightAmbient[1], kNoLightAmbient[2]};
    float   specularStrength   = 0.0f;
    float   shininess          = 1.0f;

    for (const GameObject& gameObject : scene.GetGameObjects())
    {
        const LightComponent* light = gameObject.GetComponent<LightComponent>();
        if (!light)
        {
            continue;
        }
        if (light->type != LightType::Directional)
        {
            continue;
        }
        Vector3 dir = Vector3::Transform(kAxisForward, gameObject.GetTransform().rotation);
        dir.Normalize();
        directionWorld     = dir;
        colorPremultiplied = light->color * light->intensity;
        ambientColor       = light->ambientColor;
        specularStrength   = light->specularStrength;
        shininess          = light->shininess;
        break;
    }

    DirectionalLightConstants data = {};
    data.lightDirection = Vector4(directionWorld.x, directionWorld.y, directionWorld.z, 0.0f);
    data.lightColor     = Vector4(colorPremultiplied.x, colorPremultiplied.y, colorPremultiplied.z, 0.0f);
    data.ambientColor   = Vector4(ambientColor.x, ambientColor.y, ambientColor.z, 0.0f);
    data.specularParams = Vector4(specularStrength, shininess, 0.0f, 0.0f);
    return data;
}

void DrawNode(
    ID3D11DeviceContext* ctx,
    const Model& model,
    int nodeIndex,
    const Matrix& parentAccumulated,
    const Matrix& objectWorld,
    ID3D11Buffer* modelConstantBuffer)
{
    // Row-vector convention (see ModelLoader.cpp ComputeNodeLocalTransform): v' = v * (local * parent).
    const Node& node = model.nodes[nodeIndex];
    Matrix accumulated = node.localTransform * parentAccumulated;

    if (node.meshIndex >= 0)
    {
        Matrix finalWorld = accumulated * objectWorld;
        Matrix worldInverseTranspose = finalWorld.Invert().Transpose();
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

            ModelConstants modelCb = {};
            modelCb.worldMatrix = finalWorld;
            modelCb.worldInverseTransposeMatrix = worldInverseTranspose;
            modelCb.baseColorFactor = Vector4(baseColorFactor);
            g_graphicsDevice->UpdateConstantBuffer(modelConstantBuffer, modelCb);

            ctx->DrawIndexed(prim.indexCount, 0, 0);
        }
    }

    for (int childIndex : node.childIndices)
    {
        DrawNode(ctx, model, childIndex, accumulated, objectWorld, modelConstantBuffer);
    }
}

} // namespace

void SceneRenderer::Initialize()
{
    BA_ASSERT(g_graphicsDevice);

    m_device = g_graphicsDevice->GetDevice();
    m_deviceContext = g_graphicsDevice->GetDeviceContext();

    CompileShaders();
    CreateConstantBuffers();
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
    m_directionalLightConstantBuffer.Reset();
    m_frameConstantBuffer.Reset();
    m_modelConstantBuffer.Reset();
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
    m_deviceContext->PSSetSamplers(0, 1, m_linearWrapSampler.GetAddressOf());

    ID3D11Buffer* vsBuffers[] = {
        m_modelConstantBuffer.Get(),
        m_frameConstantBuffer.Get(),
    };
    m_deviceContext->VSSetConstantBuffers(0, _countof(vsBuffers), vsBuffers);

    ID3D11Buffer* psBuffers[] = {
        m_modelConstantBuffer.Get(),
        m_frameConstantBuffer.Get(),
        m_directionalLightConstantBuffer.Get(),
    };
    m_deviceContext->PSSetConstantBuffers(0, _countof(psBuffers), psBuffers);

    FrameConstants frameCb = BuildFrameConstants(
        g_camera->GetViewMatrix(),
        g_camera->GetProjectionMatrix(aspect),
        g_camera->GetSettings().position,
        m_viewMode);
    g_graphicsDevice->UpdateConstantBuffer(m_frameConstantBuffer.Get(), frameCb);

    DirectionalLightConstants lightCb = BuildDirectionalLightConstants(*g_scene);
    g_graphicsDevice->UpdateConstantBuffer(m_directionalLightConstantBuffer.Get(), lightCb);

    for (const GameObject& gameObject : g_scene->GetGameObjects())
    {
        const ModelComponent* modelComponent = gameObject.GetComponent<ModelComponent>();
        if (!modelComponent)
        {
            continue;
        }
        const Model* model = g_modelLibrary->FindModel(modelComponent->modelName);
        BA_ASSERT(model);

        Matrix objectWorld = BuildWorld(gameObject.GetTransform());
        for (int rootIndex : model->rootNodeIndices)
        {
            DrawNode(m_deviceContext, *model, rootIndex,
                     Matrix::Identity, objectWorld,
                     m_modelConstantBuffer.Get());
        }
    }
}

ViewMode SceneRenderer::GetViewMode() const
{
    return m_viewMode;
}

void SceneRenderer::SetViewMode(ViewMode mode)
{
    m_viewMode = mode;
}

void SceneRenderer::CreateConstantBuffers()
{
    m_modelConstantBuffer            = g_graphicsDevice->CreateConstantBuffer(sizeof(ModelConstants));
    m_frameConstantBuffer            = g_graphicsDevice->CreateConstantBuffer(sizeof(FrameConstants));
    m_directionalLightConstantBuffer = g_graphicsDevice->CreateConstantBuffer(sizeof(DirectionalLightConstants));
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
