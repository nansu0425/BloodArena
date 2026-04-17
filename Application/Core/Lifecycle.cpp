#include "Core/PCH.h"
#include "Core/Lifecycle.h"
#include "Core/Settings.h"
#include "Core/Window.h"
#include "Core/Time.h"
#include "Core/Input.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/SceneRenderer.h"
#include "Graphics/MeshLibrary.h"
#include "Graphics/TextureLibrary.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"

#ifdef BA_EDITOR
#include "Graphics/SceneViewport.h"
#include "Editor/EditorUI.h"
#include "Graphics/EditorRenderer.h"
#endif // BA_EDITOR

namespace BA
{

namespace
{

void LoadTestScene()
{
    g_meshLibrary->LoadMesh("duck", "Assets/Models/Duck.glb");
    g_meshLibrary->LoadMesh("box_vertex_colors", "Assets/Models/BoxVertexColors.glb");

    uint32_t cubeId = g_scene->CreateGameObject();
    GameObject* cube = g_scene->FindGameObject(cubeId);
    cube->meshName = "cube";
    cube->transform.position = {-3.0f, 0.0f, 0.0f};
    cube->color[0] = 1.0f;
    cube->color[1] = 0.3f;
    cube->color[2] = 0.3f;
    cube->color[3] = 1.0f;

    uint32_t duckId = g_scene->CreateGameObject();
    GameObject* duck = g_scene->FindGameObject(duckId);
    duck->meshName = "duck";
    duck->transform.position = {0.0f, 0.0f, 0.0f};
    duck->transform.scale = {0.02f, 0.02f, 0.02f};
    duck->color[0] = 1.0f;
    duck->color[1] = 1.0f;
    duck->color[2] = 1.0f;
    duck->color[3] = 1.0f;

    uint32_t boxId = g_scene->CreateGameObject();
    GameObject* box = g_scene->FindGameObject(boxId);
    box->meshName = "box_vertex_colors";
    box->transform.position = {3.0f, 0.0f, 0.0f};
    box->color[0] = 0.3f;
    box->color[1] = 1.0f;
    box->color[2] = 0.3f;
    box->color[3] = 1.0f;
}

void RenderFrame()
{
    g_graphicsDevice->BeginFrame();

#ifdef BA_EDITOR
    g_editorRenderer->Render();
#else
    // TODO: When game modes are added, the game build will be locked to gameplay state
    g_sceneRenderer->Render(g_graphicsDevice->GetAspectRatio());
#endif // BA_EDITOR

    g_graphicsDevice->EndFrame();
}

void OnResize(UINT width, UINT height)
{
    g_graphicsDevice->Resize(width, height);
    RenderFrame();
}

} // namespace

void Initialize(HINSTANCE hInstance, int nShowCmd)
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
    // _CrtSetBreakAlloc(allocation number); // Break at the specified allocation number during memory allocation
#endif // _DEBUG

    g_logger = std::make_unique<Logger>();
    g_logger->Initialize();

    AppSettings appSettings = LoadSettings();

    g_time = std::make_unique<Time>();
    g_time->Initialize();

    g_input = std::make_unique<Input>();
    g_input->Initialize();

    g_window = std::make_unique<Window>();
    g_window->Initialize(hInstance, nShowCmd, appSettings.window);

    g_graphicsDevice = std::make_unique<GraphicsDevice>();
    g_graphicsDevice->Initialize(g_window->GetHandle());

    g_textureLibrary = std::make_unique<TextureLibrary>();
    g_textureLibrary->Initialize();

    g_meshLibrary = std::make_unique<MeshLibrary>();
    g_meshLibrary->Initialize();

    g_scene = std::make_unique<Scene>();
    g_scene->Initialize();

    LoadTestScene();

    g_camera = std::make_unique<Camera>();
    g_camera->Initialize(appSettings.camera);

    g_sceneRenderer = std::make_unique<SceneRenderer>();
    g_sceneRenderer->Initialize();

#ifdef BA_EDITOR
    g_sceneViewport = std::make_unique<SceneViewport>();
    g_sceneViewport->Initialize();

    g_editorUI = std::make_unique<EditorUI>();
    g_editorUI->Initialize();
    g_editorUI->SetEditorSettings(appSettings.editor);

    g_editorRenderer = std::make_unique<EditorRenderer>();
    g_editorRenderer->Initialize();
#endif // BA_EDITOR

    g_window->SetResizeCallback(OnResize);
}

int Run()
{
    MSG msg = {};
    for (;;)
    {
        g_input->BeginFrame();

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg); // Call WndProc
        }

        if (msg.message == WM_QUIT)
        {
            break;
        }

        g_time->Tick();
        g_camera->Update(g_time->GetDeltaSeconds());
        RenderFrame();
    }

    return static_cast<int>(msg.wParam);
}

void Shutdown()
{
    AppSettings appSettings;
    appSettings.window = g_window->GetSettings();
    appSettings.camera = g_camera->GetSettings();
#ifdef BA_EDITOR
    appSettings.editor = g_editorUI->GetEditorSettings();
#endif // BA_EDITOR
    SaveSettings(appSettings);

#ifdef BA_EDITOR
    g_editorRenderer->Shutdown();
    g_editorRenderer.reset();

    g_editorUI->Shutdown();
    g_editorUI.reset();

    g_sceneViewport->Shutdown();
    g_sceneViewport.reset();
#endif // BA_EDITOR

    g_sceneRenderer->Shutdown();
    g_sceneRenderer.reset();

    g_camera->Shutdown();
    g_camera.reset();

    g_scene->Shutdown();
    g_scene.reset();

    g_meshLibrary->Shutdown();
    g_meshLibrary.reset();

    g_textureLibrary->Shutdown();
    g_textureLibrary.reset();

    g_graphicsDevice->Shutdown();
    g_graphicsDevice.reset();

    g_window->Shutdown();
    g_window.reset();

    g_input->Shutdown();
    g_input.reset();

    g_time->Shutdown();
    g_time.reset();

    g_logger->Shutdown();
    g_logger.reset();
}

} // namespace BA
