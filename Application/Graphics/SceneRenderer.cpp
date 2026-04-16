#include "Core/PCH.h"
#include "Graphics/SceneRenderer.h"
#include "Graphics/GraphicsDevice.h"
#include "Math/MathUtils.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"

namespace BA
{

using namespace Microsoft::WRL;

struct Vertex
{
    Vector3 position;
};

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

    CreateSharedMesh();
    CompileShaders();
    CreateConstantBuffer();

    BA_LOG_INFO("SceneRenderer initialized.");
}

void SceneRenderer::Shutdown()
{
    m_constantBuffer.Reset();
    m_inputLayout.Reset();
    m_pixelShader.Reset();
    m_vertexShader.Reset();
    m_vertexBuffer.Reset();
    m_deviceContext = nullptr;
    m_device = nullptr;

    BA_LOG_INFO("SceneRenderer shutdown.");
}

void SceneRenderer::Render(float aspect)
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_deviceContext->IASetInputLayout(m_inputLayout.Get());
    m_deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    m_deviceContext->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    Matrix viewMatrix = g_camera->GetViewMatrix();
    Matrix projectionMatrix = g_camera->GetProjectionMatrix(aspect);

    for (const GameObject& gameObject : g_scene->GetGameObjects())
    {
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
        m_deviceContext->Draw(3, 0);
    }
}

void SceneRenderer::CreateSharedMesh()
{
    BA_ASSERT(m_device);

    Vertex vertices[] =
    {
        {{ 0.0f,   0.06f, 0.0f}},
        {{ 0.06f, -0.04f, 0.0f}},
        {{-0.06f, -0.04f, 0.0f}},
    };
    static_assert(sizeof(Vertex) == sizeof(float) * 3);

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

ComPtr<ID3DBlob> SceneRenderer::CompileShader(const wchar_t* filePath, const char* target)
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

void SceneRenderer::CreateInputLayout(ID3DBlob* vsBlob)
{
    BA_ASSERT(m_device);
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

std::unique_ptr<SceneRenderer> g_sceneRenderer;

} // namespace BA
