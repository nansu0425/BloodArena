#pragma once
#if defined(BA_EDITOR)

#include "Math/MathTypes.h"
#include "Graphics/ShadowDebug.h"

namespace BA
{

inline constexpr float kDebugFrustumFaceAlpha = 0.2f;
inline constexpr float kDebugFrustumLineAlpha = 1.0f;

class DebugRenderer
{
public:
    void Initialize();
    void Shutdown();

    // Renders a unit NDC cube transformed by the inverse of `frustumViewProjection`
    // into world space, then projected with the editor camera's view/projection.
    // Re-binds RTV+DSV explicitly to avoid implicit OM-state coupling. Depth-tested
    // against the bound DSV (test on, write off); face is alpha-blended, edges are opaque.
    void DrawFrustum(const Matrix&  frustumViewProjection,
                     const Matrix&  cameraView,
                     const Matrix&  cameraProjection,
                     const Vector3& color,
                     ID3D11RenderTargetView* rtv,
                     ID3D11DepthStencilView* dsv,
                     UINT viewportWidth,
                     UINT viewportHeight);

    ShadowDebugSettings GetShadowDebugSettings() const;
    void                SetShadowDebugSettings(const ShadowDebugSettings& settings);

    // Fullscreen alpha-blend overlay onto rtv. Reconstructs world position from sceneDepthSrv,
    // computes shadow uv from the shadow pass matrices, then draws the active debug visualization.
    // Caller must skip the call when settings.mode == ShadowDebugMode::Off.
    void DrawShadowDebugOverlay(ID3D11ShaderResourceView* sceneDepthSrv,
                                const Matrix&             cameraView,
                                const Matrix&             cameraProjection,
                                const Matrix&             lightViewMatrix,
                                const Matrix&             lightProjectionMatrix,
                                ID3D11RenderTargetView*   rtv,
                                UINT                      viewportWidth,
                                UINT                      viewportHeight);

private:
    void CompileShaders();
    void CreateInputLayout(ID3DBlob* vsBlob);
    void CreateCubeBuffers();
    void CreateConstantBuffer();
    void CreateRasterizerStates();
    void CreateDepthState();
    void CreateBlendStates();

    void CompileShadowOverlayShaders();
    void CreateShadowOverlayConstantBuffer();
    void CreateShadowOverlayDepthState();
    void CreateShadowOverlaySampler();

    Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const wchar_t* filePath, const char* target);

private:
    ID3D11Device*        m_device        = nullptr;
    ID3D11DeviceContext* m_deviceContext = nullptr;

    Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_cubeVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_cubeFaceIndexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_cubeLineIndexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_constantBuffer;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>   m_rasterizerState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthState;
    Microsoft::WRL::ComPtr<ID3D11BlendState>        m_alphaBlendState;
    Microsoft::WRL::ComPtr<ID3D11BlendState>        m_opaqueBlendState;

    Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_shadowOverlayVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_shadowOverlayPixelShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_shadowOverlayConstantBuffer;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_shadowOverlayDepthState;
    Microsoft::WRL::ComPtr<ID3D11SamplerState>      m_shadowOverlaySampler;

    ShadowDebugSettings m_shadowDebugSettings;
};

extern std::unique_ptr<DebugRenderer> g_debugRenderer;

} // namespace BA

#endif // BA_EDITOR
