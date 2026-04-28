#include "Core/PCH.h"
#include "Graphics/EditorRenderer.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/ModelLibrary.h"
#include "Graphics/SceneRenderer.h"
#include "Graphics/SceneViewport.h"
#include "Graphics/ShadowFrustum.h"
#include "Graphics/ShadowMap.h"
#include "Graphics/DebugRenderer.h"
#include "Editor/EditorState.h"
#include "Editor/ViewportPicking.h"
#include "Graphics/Gizmo/Gizmo.h"
#include "Core/Window.h"
#include "Core/Input.h"
#include "Core/PathUtils.h"
#include "Core/PlaySession.h"
#include "Scene/Scene.h"
#include "Editor/EditorCamera.h"
#include "Scene/GameObject.h"
#include "Math/MathUtils.h"

#include <filesystem>

#pragma warning(push, 0)
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#pragma warning(pop)

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace BA
{

namespace
{

LRESULT ImGuiWndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
}

ImVec4 GetLogLevelColor(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Trace:    return ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    case LogLevel::Debug:    return ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    case LogLevel::Info:     return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    case LogLevel::Warn:     return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    case LogLevel::Error:    return ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
    case LogLevel::Critical: return ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
    default:                 return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

const char* kLogLevelNames[] = { "Trace", "Debug", "Info", "Warn", "Error", "Critical" };

const char* kShadowDebugModeLabels[] = { "Off", "Texel Grid", "PPT Heatmap" };

const Vector3 kCameraFrustumVisualizationColor = { 0.2f, 0.8f, 1.0f };

bool HasModelComponent(const GameObject& gameObject)
{
    return gameObject.HasComponent<ModelComponent>();
}

void AddModelComponent(GameObject& gameObject)
{
    BA_ASSERT(g_modelLibrary->FindModel(kDefaultModelName));
    gameObject.AddComponent<ModelComponent>(kDefaultModelName);
}

bool HasLightComponent(const GameObject& gameObject)
{
    return gameObject.HasComponent<LightComponent>();
}

void AddLightComponent(GameObject& gameObject)
{
    gameObject.AddComponent<LightComponent>();
}

bool HasCameraComponent(const GameObject& gameObject)
{
    return gameObject.HasComponent<CameraComponent>();
}

void AddCameraComponent(GameObject& gameObject)
{
    gameObject.AddComponent<CameraComponent>();
}

struct ComponentAddEntry
{
    const char* displayName;
    bool (*isPresent)(const GameObject&);
    void (*add)(GameObject&);
};

constexpr ComponentAddEntry kComponentAddEntries[] = {
    { "Model",  &HasModelComponent,  &AddModelComponent  },
    { "Light",  &HasLightComponent,  &AddLightComponent  },
    { "Camera", &HasCameraComponent, &AddCameraComponent },
};

constexpr const char* kAddComponentPopupId = "AddComponentPopup";

constexpr float kInspectorRotationDragSpeedDeg = 0.1f;

constexpr const char* kViewModeLabels[] = { "Lit", "Unlit" };

constexpr float kViewModeComboWidth = 80.0f;

constexpr float kLightArrowScreenLength = 0.15f;

const char* GizmoModeToLabel(Gizmo::Mode mode)
{
    switch (mode)
    {
    case Gizmo::Mode::None:
    {
        return "None";
    }
    case Gizmo::Mode::Translate:
    {
        return "Translate";
    }
    case Gizmo::Mode::Rotate:
    {
        return "Rotate";
    }
    case Gizmo::Mode::Scale:
    {
        return "Scale";
    }
    }
    BA_ASSERT(false);
    return "Unknown";
}

const char* GizmoSpaceToLabel(Gizmo::Space space)
{
    switch (space)
    {
    case Gizmo::Space::World:
    {
        return "World";
    }
    case Gizmo::Space::Local:
    {
        return "Local";
    }
    }
    BA_ASSERT(false);
    return "Unknown";
}

struct InspectorAxisDrag
{
    bool    isDragging;
    Vector3 axis;
    float   deltaDeg;
};

InspectorAxisDrag GetDraggedAxis(const Vector3& deltaDeg)
{
    if (deltaDeg.x != 0.0f)
    {
        return { true, kAxisRight,   deltaDeg.x };
    }
    if (deltaDeg.y != 0.0f)
    {
        return { true, kAxisUp,      deltaDeg.y };
    }
    if (deltaDeg.z != 0.0f)
    {
        return { true, kAxisForward, deltaDeg.z };
    }

    return { false, Vector3::Zero, 0.0f };
}

} // namespace

void EditorRenderer::Initialize()
{
    BA_ASSERT(g_window);
    BA_ASSERT(g_graphicsDevice);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(g_window->GetHandle());
    ImGui_ImplDX11_Init(
        g_graphicsDevice->GetDevice(),
        g_graphicsDevice->GetDeviceContext()
    );

    g_window->SetEditorWndProc(ImGuiWndProcHandler);

    BA_LOG_INFO("EditorRenderer initialized.");
}

void EditorRenderer::Shutdown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    BA_LOG_INFO("EditorRenderer shutdown.");
}

void EditorRenderer::BeginImGuiFrame()
{
    BA_ASSERT(m_framePhase == FramePhase::Idle);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    Gizmo::BeginFrame();

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_None);

    m_framePhase = FramePhase::ImGuiFrameBegun;
}

void EditorRenderer::ResolveViewportInput()
{
    BA_ASSERT(m_framePhase == FramePhase::ImGuiFrameBegun);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_MenuBar);
    ImGui::PopStyleVar();

    const bool wasViewportFlying = g_editorState->IsViewportFlying();
    const bool isViewportHovered = ImGui::IsWindowHovered();
    if (isViewportHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        g_editorState->SetViewportFlying(true);
    }
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
        g_editorState->SetViewportFlying(false);
    }

    if (!wasViewportFlying && g_editorState->IsViewportFlying())
    {
        ImGui::SetWindowFocus();
    }

    ImGui::End();

    m_framePhase = FramePhase::ViewportInputResolved;
}

void EditorRenderer::UpdateInputCapture()
{
    BA_ASSERT(m_framePhase == FramePhase::ViewportInputResolved);

    if (g_editorState->IsViewportFlying())
    {
        g_input->SetKeyboardCaptured(false);
        g_input->SetMouseCaptured(false);
    }
    else
    {
        ImGuiIO& io = ImGui::GetIO();
        g_input->SetKeyboardCaptured(true);
        g_input->SetMouseCaptured(io.WantCaptureMouse);
    }

    m_framePhase = FramePhase::InputCaptureUpdated;
}

void EditorRenderer::RenderPanels()
{
    BA_ASSERT(m_framePhase == FramePhase::InputCaptureUpdated);

    RenderViewport();
    RenderHierarchy();
    RenderInspector();
    RenderConsole();

    m_framePhase = FramePhase::PanelsRendered;
}

void EditorRenderer::EndImGuiFrame()
{
    BA_ASSERT(m_framePhase == FramePhase::PanelsRendered);

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    m_framePhase = FramePhase::Idle;
}

void EditorRenderer::RenderViewport()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_MenuBar);
    ImGui::PopStyleVar();

    if (ImGui::BeginMenuBar())
    {
        const PlayMode playMode = g_playSession->GetMode();
        const bool canPlay  = (playMode != PlayMode::Playing);
        const bool canPause = (playMode == PlayMode::Playing);
        const bool canStop  = (playMode != PlayMode::Edit);

        ImGui::BeginDisabled(!canPlay);
        if (ImGui::Button("Play"))
        {
            if (playMode == PlayMode::Edit)
            {
                g_playSession->StartPlay();
            }
            else
            {
                g_playSession->Resume();
            }
        }
        ImGui::EndDisabled();
        ImGui::SameLine();

        ImGui::BeginDisabled(!canPause);
        if (ImGui::Button("Pause"))
        {
            g_playSession->Pause();
        }
        ImGui::EndDisabled();
        ImGui::SameLine();

        ImGui::BeginDisabled(!canStop);
        if (ImGui::Button("Stop"))
        {
            g_playSession->Stop();
        }
        ImGui::EndDisabled();
        ImGui::SameLine();

        ImGui::TextUnformatted("|");
        ImGui::SameLine();

        ImGui::TextUnformatted(GizmoModeToLabel(g_editorState->GetGizmoMode()));
        ImGui::SameLine();
        ImGui::TextUnformatted("|");
        ImGui::SameLine();
        ImGui::TextUnformatted(GizmoSpaceToLabel(g_editorState->GetGizmoSpace()));
        ImGui::SameLine();

        const char* kCameraLabel = "Camera";
        float cameraButtonWidth = ImGui::CalcTextSize(kCameraLabel).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float itemSpacing = ImGui::GetStyle().ItemSpacing.x;
        float rightBlockWidth = kViewModeComboWidth + itemSpacing + cameraButtonWidth;
        float available = ImGui::GetContentRegionAvail().x;
        if (available > rightBlockWidth)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available - rightBlockWidth);
        }

        int viewModeIndex = static_cast<int>(g_sceneRenderer->GetViewMode());
        ImGui::SetNextItemWidth(kViewModeComboWidth);
        if (ImGui::Combo("##ViewMode", &viewModeIndex, kViewModeLabels, IM_ARRAYSIZE(kViewModeLabels)))
        {
            g_sceneRenderer->SetViewMode(static_cast<ViewMode>(viewModeIndex));
        }
        ImGui::SameLine();

        if (ImGui::Button(kCameraLabel))
        {
            ImGui::OpenPopup("CameraSettingsPopup");
        }
        if (ImGui::BeginPopup("CameraSettingsPopup"))
        {
            RenderCameraSettingsMenu();
            ImGui::EndPopup();
        }
        ImGui::EndMenuBar();
    }

    ImVec2 size = ImGui::GetContentRegionAvail();
    UINT width = static_cast<UINT>(size.x);
    UINT height = static_cast<UINT>(size.y);

    if (width > 0 && height > 0)
    {
        if (width != g_sceneViewport->GetWidth() || height != g_sceneViewport->GetHeight())
        {
            g_sceneViewport->Resize(width, height);
        }

        Matrix  view;
        Matrix  proj;
        Vector3 position;

        if (g_playSession->GetMode() == PlayMode::Edit)
        {
            EditorCamera* editorCamera = g_sceneViewport->GetEditorCamera();
            BA_ASSERT(editorCamera);

            view     = editorCamera->GetViewMatrix();
            proj     = editorCamera->GetProjectionMatrix();
            position = editorCamera->GetPosition();
        }
        else
        {
            const ActiveCameraResult active = g_scene->FindActiveCamera();
            BA_ASSERT(active.isFound);

            const float aspect = static_cast<float>(g_sceneViewport->GetWidth())
                               / static_cast<float>(g_sceneViewport->GetHeight());
            active.camera->SetAspect(aspect);

            view     = active.camera->GetViewMatrix(active.owner->GetTransform());
            proj     = active.camera->GetProjectionMatrix();
            position = active.owner->GetTransform().position;
        }

        g_sceneRenderer->RenderShadowPass(*g_scene, view, proj);
        g_sceneViewport->Clear();

        g_sceneRenderer->RenderMainPass(*g_scene, view, proj, position);

        const ShadowDebugSettings shadowDebugSettings = g_debugRenderer->GetShadowDebugSettings();
        if (shadowDebugSettings.mode != ShadowDebugMode::Off
            && g_sceneRenderer->IsLastShadowEnabled())
        {
            g_debugRenderer->DrawShadowDebugOverlay(
                g_sceneViewport->GetDepthSRV(),
                view,
                proj,
                g_sceneRenderer->GetLastShadowLightViewMatrix(),
                g_sceneRenderer->GetLastShadowLightProjectionMatrix(),
                g_sceneViewport->GetRTV(),
                g_sceneViewport->GetWidth(),
                g_sceneViewport->GetHeight());
        }

        for (const GameObject& gameObject : g_scene->GetGameObjects())
        {
            const LightComponent* light = gameObject.GetComponent<LightComponent>();
            if (!light || !light->IsEnabled())
            {
                continue;
            }
            if (light->GetType() != LightType::Directional)
            {
                continue;
            }
            if (!light->ShouldCastShadow())
            {
                continue;
            }

            if (light->IsShadowFrustumVisualized())
            {
                const DirectionalShadowFrustum frustum = ComputeDirectionalShadowFrustum(
                    *light, gameObject.GetTransform());
                const Matrix lightViewProj = frustum.lightViewMatrix * frustum.lightProjectionMatrix;

                g_debugRenderer->DrawFrustum(
                    lightViewProj,
                    view,
                    proj,
                    light->GetColor(),
                    g_sceneViewport->GetRTV(),
                    g_sceneViewport->GetDSV(),
                    g_sceneViewport->GetWidth(),
                    g_sceneViewport->GetHeight());
            }
            break;
        }

        for (const GameObject& gameObject : g_scene->GetGameObjects())
        {
            const CameraComponent* camera = gameObject.GetComponent<CameraComponent>();
            if (!camera || !camera->IsEnabled())
            {
                continue;
            }
            if (!camera->IsViewFrustumVisualized())
            {
                continue;
            }

            const Matrix cameraView       = camera->GetViewMatrix(gameObject.GetTransform());
            const Matrix cameraProjection = camera->GetProjectionMatrix();
            const Matrix cameraViewProj   = cameraView * cameraProjection;

            g_debugRenderer->DrawFrustum(
                cameraViewProj,
                view,
                proj,
                kCameraFrustumVisualizationColor,
                g_sceneViewport->GetRTV(),
                g_sceneViewport->GetDSV(),
                g_sceneViewport->GetWidth(),
                g_sceneViewport->GetHeight());
        }

        g_graphicsDevice->RestoreBackBuffer();

        ImGui::Image(
            reinterpret_cast<ImTextureID>(g_sceneViewport->GetSRV()),
            size
        );

        if (!ImGui::IsMouseDown(ImGuiMouseButton_Right) && !ImGui::GetIO().WantTextInput)
        {
            if (ImGui::IsKeyPressed(ImGuiKey_W, false))
            {
                g_editorState->SetGizmoMode(Gizmo::Mode::Translate);
            }
            if (ImGui::IsKeyPressed(ImGuiKey_E, false))
            {
                g_editorState->SetGizmoMode(Gizmo::Mode::Rotate);
            }
            if (ImGui::IsKeyPressed(ImGuiKey_R, false))
            {
                g_editorState->SetGizmoMode(Gizmo::Mode::Scale);
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Q, false))
            {
                g_editorState->SetGizmoMode(Gizmo::Mode::None);
            }
            if (ImGui::IsKeyPressed(ImGuiKey_X, false))
            {
                const Gizmo::Space currentSpace = g_editorState->GetGizmoSpace();
                const Gizmo::Space toggledSpace = (currentSpace == Gizmo::Space::World)
                    ? Gizmo::Space::Local
                    : Gizmo::Space::World;
                g_editorState->SetGizmoSpace(toggledSpace);
            }
        }

        ImVec2 rectMin = ImGui::GetItemRectMin();
        ImVec2 rectSize = ImGui::GetItemRectSize();

        for (const GameObject& gameObject : g_scene->GetGameObjects())
        {
            const LightComponent* light = gameObject.GetComponent<LightComponent>();
            if (!light || !light->IsEnabled())
            {
                continue;
            }
            if (light->GetType() != LightType::Directional)
            {
                continue;
            }
            const Transform& tf = gameObject.GetTransform();
            Vector3 direction = Vector3::Transform(kAxisForward, tf.rotation);
            direction.Normalize();
            Gizmo::DrawArrow(
                tf.position,
                direction,
                kLightArrowScreenLength,
                light->GetColor(),
                rectMin.x, rectMin.y, rectSize.x, rectSize.y,
                view, proj);
        }

        GameObject* selected = g_scene->FindGameObject(g_editorState->GetSelectedGameObjectId());
        if (selected != nullptr && g_editorState->GetGizmoMode() != Gizmo::Mode::None)
        {
            Gizmo::SetViewportRect(rectMin.x, rectMin.y, rectSize.x, rectSize.y);

            Gizmo::ManipulateResult result = Gizmo::Manipulate(
                selected->GetTransform(),
                g_editorState->GetGizmoMode(),
                g_editorState->GetGizmoSpace(),
                view,
                proj
            );
            if (result.isChanged)
            {
                selected->SetTransform(result.transform);
            }
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !Gizmo::IsUsingMouse())
        {
            ImVec2 imageMin = ImGui::GetItemRectMin();
            ImVec2 mousePos = ImGui::GetMousePos();

            float pixelX = mousePos.x - imageMin.x;
            float pixelY = mousePos.y - imageMin.y;

            float ndcX = (pixelX / size.x) * 2.0f - 1.0f;
            float ndcY = 1.0f - (pixelY / size.y) * 2.0f;

            uint32_t hitId = PickGameObject(ndcX, ndcY, view, proj);
            g_editorState->SetSelectedGameObjectId(hitId);
        }
    }

    ImGui::End();
}

void EditorRenderer::RenderCameraSettingsMenu()
{
    EditorCamera* editorCamera = g_sceneViewport->GetEditorCamera();
    BA_ASSERT(editorCamera);

    EditorCameraSettings settings = editorCamera->GetSettings();

    ImGui::PushItemWidth(180.0f);

    ImGui::DragFloat3("Position", &settings.position.x, 0.1f);

    float yawDeg = RadToDeg(settings.yaw);
    float pitchDeg = RadToDeg(settings.pitch);
    if (ImGui::DragFloat("Yaw", &yawDeg, 0.5f))
    {
        settings.yaw = DegToRad(yawDeg);
    }
    if (ImGui::DragFloat("Pitch", &pitchDeg, 0.5f, -89.0f, 89.0f))
    {
        settings.pitch = DegToRad(pitchDeg);
    }

    float fovDeg = RadToDeg(settings.fovY);
    if (ImGui::SliderFloat("FOV", &fovDeg, 10.0f, 120.0f, "%.1f deg"))
    {
        settings.fovY = DegToRad(fovDeg);
    }

    ImGui::DragFloat("Near Z", &settings.nearZ, 0.01f, 0.001f, settings.farZ);
    ImGui::DragFloat("Far Z", &settings.farZ, 1.0f, settings.nearZ, 100000.0f);
    ImGui::DragFloat("Move Speed", &settings.moveSpeed, 0.1f, 0.0f, 1000.0f);
    ImGui::DragFloat("Mouse Sensitivity", &settings.mouseSensitivity, 0.0001f, 0.0f, 1.0f, "%.4f");

    ImGui::PopItemWidth();

    editorCamera->SetSettings(settings);

    ImGui::Separator();
    if (ImGui::Button("Reset"))
    {
        editorCamera->ResetToDefaults();
    }
}

void EditorRenderer::RenderHierarchy()
{
    ImGui::Begin("Hierarchy");

    uint32_t selectedId = g_editorState->GetSelectedGameObjectId();

    if (ImGui::Button("Create"))
    {
        uint32_t newId = g_scene->CreateGameObject();
        g_editorState->SetSelectedGameObjectId(newId);
        selectedId = newId;
    }

    ImGui::SameLine();

    const bool hasSelection = (selectedId != 0) && (g_scene->FindGameObject(selectedId) != nullptr);
    ImGui::BeginDisabled(!hasSelection);
    if (ImGui::Button("Remove"))
    {
        g_scene->DestroyGameObject(selectedId);
        g_editorState->SetSelectedGameObjectId(0);
        selectedId = 0;
    }
    ImGui::EndDisabled();

    ImGui::Separator();

    if (ImGui::Button("New Scene"))
    {
        g_scene->Clear();
        g_sceneViewport->GetEditorCamera()->ResetToDefaults();
        g_editorState->SetSelectedGameObjectId(0);
        selectedId = 0;
        g_editorState->ClearSceneNameBuffer();
    }
    ImGui::SameLine();
    if (ImGui::Button("Save Scene"))
    {
        ImGui::OpenPopup("SaveScenePopup");
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Scene"))
    {
        ImGui::OpenPopup("LoadScenePopup");
    }

    if (ImGui::BeginPopup("SaveScenePopup"))
    {
        ImGui::InputText("Name", g_editorState->GetSceneNameBuffer(), kEditorSceneNameBufferSize);

        const bool hasSceneName = (g_editorState->GetSceneNameBuffer()[0] != '\0');
        ImGui::BeginDisabled(!hasSceneName);
        if (ImGui::Button("Save"))
        {
            g_scene->SaveToFile(g_editorState->GetSceneNameBuffer());
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();

        ImGui::Separator();
        ImGui::TextDisabled("Overwrite existing:");

        std::wstring scenesDir = ResolveAssetPath(L"Assets/Scenes");
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(scenesDir, ec))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }
            if (entry.path().extension() != ".json")
            {
                continue;
            }

            std::string stem = entry.path().stem().string();
            if (ImGui::Selectable(stem.c_str()))
            {
                g_scene->SaveToFile(stem);
                g_editorState->SetSceneNameBuffer(stem.c_str());
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("LoadScenePopup"))
    {
        std::wstring scenesDir = ResolveAssetPath(L"Assets/Scenes");
        std::error_code ec;
        bool hasAnyScene = false;
        for (const auto& entry : std::filesystem::directory_iterator(scenesDir, ec))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }
            if (entry.path().extension() != ".json")
            {
                continue;
            }
            hasAnyScene = true;

            std::string stem = entry.path().stem().string();
            if (ImGui::Selectable(stem.c_str()))
            {
                if (g_scene->LoadFromFile(stem))
                {
                    g_editorState->SetSceneNameBuffer(stem.c_str());
                    g_editorState->SetSelectedGameObjectId(0);
                    selectedId = 0;
                }
                ImGui::CloseCurrentPopup();
            }
        }
        if (!hasAnyScene)
        {
            ImGui::TextDisabled("No scenes found");
        }
        ImGui::EndPopup();
    }

    ImGui::Separator();

    for (const GameObject& gameObject : g_scene->GetGameObjects())
    {
        bool isSelected = (selectedId == gameObject.GetId());

        char label[96];
        snprintf(label, sizeof(label), "%s##%u", gameObject.GetName().c_str(), gameObject.GetId());

        if (ImGui::Selectable(label, isSelected))
        {
            g_editorState->SetSelectedGameObjectId(gameObject.GetId());
        }
    }

    ImGui::End();
}

void EditorRenderer::RenderInspector()
{
    ImGui::Begin("Inspector");

    uint32_t selectedId = g_editorState->GetSelectedGameObjectId();

    if (selectedId == 0)
    {
        ImGui::TextDisabled("No object selected");
        ImGui::End();
        return;
    }

    GameObject* selected = g_scene->FindGameObject(selectedId);
    if (selected == nullptr)
    {
        g_editorState->SetSelectedGameObjectId(0);
        ImGui::TextDisabled("No object selected");
        ImGui::End();
        return;
    }

    ImGui::Text("ID: %u", selected->GetId());

    char nameBuffer[kGameObjectNameBufferSize] = {};
    selected->GetName().copy(nameBuffer, kGameObjectNameBufferSize - 1);
    if (ImGui::InputText("Name", nameBuffer, kGameObjectNameBufferSize))
    {
        selected->SetName(nameBuffer);
    }

    ImGui::Separator();

    Transform& transform = selected->GetTransform();

    Vector3 positionEdit = transform.position;
    ImGui::DragFloat3("Position", &positionEdit.x, 0.01f);
    const Vector3 deltaPosition = positionEdit - transform.position;
    const bool hasPositionDelta =
        (deltaPosition.x != 0.0f) || (deltaPosition.y != 0.0f) || (deltaPosition.z != 0.0f);
    if (hasPositionDelta)
    {
        const Vector3 worldDelta = (g_editorState->GetGizmoSpace() == Gizmo::Space::Local)
            ? Vector3::Transform(deltaPosition, transform.rotation)
            : deltaPosition;
        transform.position += worldDelta;
    }

    const Vector3 eulerRad = QuaternionToEulerZXY(transform.rotation);
    Vector3 eulerDeg = {RadToDeg(eulerRad.x), RadToDeg(eulerRad.y), RadToDeg(eulerRad.z)};
    const Vector3 previousDeg = eulerDeg;

    ImGui::DragFloat3("Rotation", &eulerDeg.x, kInspectorRotationDragSpeedDeg);

    const Vector3 deltaDeg = eulerDeg - previousDeg;
    const InspectorAxisDrag drag = GetDraggedAxis(deltaDeg);
    if (drag.isDragging)
    {
        Quaternion& rotation = transform.rotation;
        const Quaternion delta = Quaternion::CreateFromAxisAngle(drag.axis, DegToRad(drag.deltaDeg));
        const bool isLocal = (g_editorState->GetGizmoSpace() == Gizmo::Space::Local);
        rotation = isLocal ? (delta * rotation) : (rotation * delta);
        rotation.Normalize();
    }

    ImGui::DragFloat3("Scale", &transform.scale.x, 0.01f);

    ImGui::Separator();

    RenderModelComponent(*selected);

    RenderLightComponent(*selected);

    RenderCameraComponent(*selected);

    RenderAddComponentMenu(*selected);

    ImGui::End();
}

void EditorRenderer::RenderModelComponent(GameObject& gameObject)
{
    ModelComponent* modelComponent = gameObject.GetComponent<ModelComponent>();
    if (!modelComponent)
    {
        return;
    }

    if (!ImGui::CollapsingHeader("Model Component", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    bool isEnabled = modelComponent->IsEnabled();
    if (ImGui::Checkbox("Enabled##Model", &isEnabled))
    {
        modelComponent->SetEnabled(isEnabled);
    }

    ImGui::BeginDisabled(!isEnabled);

    std::vector<std::string> names = g_modelLibrary->GetModelNames();

    std::vector<const char*> items;
    items.reserve(names.size());
    for (const std::string& name : names)
    {
        items.push_back(name.c_str());
    }

    const std::string& currentName = modelComponent->GetModelName();
    int currentIndex = -1;
    for (size_t i = 0; i < names.size(); ++i)
    {
        if (names[i] == currentName)
        {
            currentIndex = static_cast<int>(i);
            break;
        }
    }
    BA_ASSERT(currentIndex >= 0);

    if (ImGui::Combo("Model", &currentIndex, items.data(), static_cast<int>(items.size())))
    {
        const std::string& selectedName = names[currentIndex];
        BA_ASSERT(g_modelLibrary->FindModel(selectedName));
        modelComponent->SetModelName(selectedName);
    }

    const Model* model = g_modelLibrary->FindModel(modelComponent->GetModelName());
    BA_ASSERT(model);

    size_t primitiveCount = 0;
    for (const Mesh& mesh : model->meshes)
    {
        primitiveCount += mesh.primitives.size();
    }

    ImGui::Text("Nodes: %zu", model->nodes.size());
    ImGui::Text("Meshes: %zu", model->meshes.size());
    ImGui::Text("Primitives: %zu", primitiveCount);
    ImGui::Text("Materials: %zu", model->materials.size());

    ImGui::EndDisabled();

    if (ImGui::Button("Remove"))
    {
        gameObject.RemoveComponent<ModelComponent>();
    }
}

void EditorRenderer::RenderLightComponent(GameObject& gameObject)
{
    LightComponent* lightComponent = gameObject.GetComponent<LightComponent>();
    if (!lightComponent)
    {
        return;
    }

    if (!ImGui::CollapsingHeader("Light Component", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    bool isEnabled = lightComponent->IsEnabled();
    if (ImGui::Checkbox("Enabled##Light", &isEnabled))
    {
        lightComponent->SetEnabled(isEnabled);
    }

    ImGui::BeginDisabled(!isEnabled);

    const char* kLightTypeLabels[] = { "Directional", "Ambient" };
    int typeIndex = static_cast<int>(lightComponent->GetType());
    if (ImGui::Combo("Type", &typeIndex, kLightTypeLabels, IM_ARRAYSIZE(kLightTypeLabels)))
    {
        lightComponent->SetType(static_cast<LightType>(typeIndex));
    }

    switch (lightComponent->GetType())
    {
    case LightType::Directional:
    {
        ImGui::TextDisabled("Direction follows Transform Rotation");

        Vector3 color = lightComponent->GetColor();
        if (ImGui::ColorEdit3("Color", &color.x))
        {
            lightComponent->SetColor(color);
        }

        float intensity = lightComponent->GetIntensity();
        if (ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 100.0f))
        {
            lightComponent->SetIntensity(intensity);
        }

        float specularStrength = lightComponent->GetSpecularStrength();
        if (ImGui::DragFloat("Specular Strength", &specularStrength, 0.01f, 0.0f, 10.0f))
        {
            lightComponent->SetSpecularStrength(specularStrength);
        }

        float shininess = lightComponent->GetShininess();
        if (ImGui::DragFloat("Shininess", &shininess, 0.5f, 1.0f, 512.0f))
        {
            lightComponent->SetShininess(shininess);
        }

        ImGui::Separator();

        bool shouldCastShadow = lightComponent->ShouldCastShadow();
        if (ImGui::Checkbox("Cast Shadow", &shouldCastShadow))
        {
            lightComponent->SetShouldCastShadow(shouldCastShadow);
        }

        ImGui::BeginDisabled(!shouldCastShadow);

        const bool hasEligibleMesh = HasAnyShadowFitMesh(*g_scene);
        const bool isAutoFit       = lightComponent->IsShadowFrustumAutoFit();

        ImGui::BeginDisabled(!hasEligibleMesh);
        bool isAutoFitToggle = isAutoFit;
        if (ImGui::Checkbox("Auto-fit Shadow Frustum", &isAutoFitToggle))
        {
            lightComponent->SetShadowFrustumAutoFit(isAutoFitToggle);
        }
        ImGui::EndDisabled();
        if (!hasEligibleMesh && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("No enabled mesh found in scene");
        }

        ImGui::BeginDisabled(isAutoFit);

        float shadowOrthoWidth = lightComponent->GetShadowOrthoWidth();
        if (ImGui::DragFloat("Shadow Ortho Width", &shadowOrthoWidth, 0.5f, 0.0f, 0.0f))
        {
            lightComponent->SetShadowOrthoWidth(shadowOrthoWidth);
        }

        float shadowOrthoHeight = lightComponent->GetShadowOrthoHeight();
        if (ImGui::DragFloat("Shadow Ortho Height", &shadowOrthoHeight, 0.5f, 0.0f, 0.0f))
        {
            lightComponent->SetShadowOrthoHeight(shadowOrthoHeight);
        }

        float shadowNearZ = lightComponent->GetShadowNearZ();
        if (ImGui::DragFloat("Shadow Near Z", &shadowNearZ, 0.1f, 0.0f, 0.0f))
        {
            lightComponent->SetShadowNearZ(shadowNearZ);
        }

        float shadowFarZ = lightComponent->GetShadowFarZ();
        if (ImGui::DragFloat("Shadow Far Z", &shadowFarZ, 0.1f, 0.0f, 0.0f))
        {
            lightComponent->SetShadowFarZ(shadowFarZ);
        }

        Vector3 shadowFrustumCenter = lightComponent->GetShadowFrustumCenter();
        if (ImGui::DragFloat3("Shadow Frustum Center", &shadowFrustumCenter.x, 0.1f))
        {
            lightComponent->SetShadowFrustumCenter(shadowFrustumCenter);
        }

        ImGui::EndDisabled();

        float shadowDepthBias = lightComponent->GetShadowDepthBias();
        if (ImGui::DragFloat("Shadow Depth Bias", &shadowDepthBias, 0.0001f, 0.0f, 0.0f, "%.4f"))
        {
            lightComponent->SetShadowDepthBias(shadowDepthBias);
        }

        ImGui::SeparatorText("Shadow Debug");

        const float orthoWidth     = lightComponent->GetShadowOrthoWidth();
        const float orthoHeight    = lightComponent->GetShadowOrthoHeight();
        const float texelWorldSize = std::max(orthoWidth, orthoHeight)
                                   / static_cast<float>(kDefaultShadowMapResolution);

        ImGui::Text("Texel Size: %.4f units (%.2f cm)",
                    texelWorldSize, texelWorldSize * 100.0f);
        ImGui::Text("Shadow Map: %ux%u",
                    kDefaultShadowMapResolution, kDefaultShadowMapResolution);

        if (g_sceneRenderer->IsLastShadowEnabled())
        {
            ID3D11ShaderResourceView* shadowSrv = g_sceneRenderer->GetShadowMap()->GetSRV();
            const float previewSize = ImGui::GetContentRegionAvail().x;
            ImGui::Image(reinterpret_cast<ImTextureID>(shadowSrv),
                         ImVec2(previewSize, previewSize));
        }

        bool isShadowFrustumVisualized = lightComponent->IsShadowFrustumVisualized();
        if (ImGui::Checkbox("Visualize Shadow Frustum", &isShadowFrustumVisualized))
        {
            lightComponent->SetShadowFrustumVisualized(isShadowFrustumVisualized);
        }

        ShadowDebugSettings shadowDebugSettings = g_debugRenderer->GetShadowDebugSettings();

        int shadowDebugModeIndex = static_cast<int>(shadowDebugSettings.mode);
        if (ImGui::Combo("Debug Mode", &shadowDebugModeIndex,
                         kShadowDebugModeLabels, IM_ARRAYSIZE(kShadowDebugModeLabels)))
        {
            shadowDebugSettings.mode = static_cast<ShadowDebugMode>(shadowDebugModeIndex);
            g_debugRenderer->SetShadowDebugSettings(shadowDebugSettings);
        }

        if (shadowDebugSettings.mode == ShadowDebugMode::PptHeatmap)
        {
            if (ImGui::DragFloat("Red Min",
                                 &shadowDebugSettings.pptThresholdRed, 0.05f, 0.0f, 0.0f, "%.2f"))
            {
                g_debugRenderer->SetShadowDebugSettings(shadowDebugSettings);
            }

            if (ImGui::DragFloat("Orange Min",
                                 &shadowDebugSettings.pptThresholdOrange, 0.05f, 0.0f, 0.0f, "%.2f"))
            {
                g_debugRenderer->SetShadowDebugSettings(shadowDebugSettings);
            }

            if (ImGui::DragFloat("Green Min",
                                 &shadowDebugSettings.pptThresholdGreen, 0.05f, 0.0f, 0.0f, "%.2f"))
            {
                g_debugRenderer->SetShadowDebugSettings(shadowDebugSettings);
            }
        }

        ImGui::EndDisabled();

        break;
    }
    case LightType::Ambient:
    {
        Vector3 color = lightComponent->GetColor();
        if (ImGui::ColorEdit3("Color", &color.x))
        {
            lightComponent->SetColor(color);
        }

        float intensity = lightComponent->GetIntensity();
        if (ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 100.0f))
        {
            lightComponent->SetIntensity(intensity);
        }

        break;
    }
    }

    ImGui::EndDisabled();

    if (ImGui::Button("Remove##Light"))
    {
        gameObject.RemoveComponent<LightComponent>();
    }
}

void EditorRenderer::RenderCameraComponent(GameObject& gameObject)
{
    CameraComponent* cameraComponent = gameObject.GetComponent<CameraComponent>();
    if (!cameraComponent)
    {
        return;
    }

    if (!ImGui::CollapsingHeader("Camera Component", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    bool isEnabled = cameraComponent->IsEnabled();
    if (ImGui::Checkbox("Enabled##Camera", &isEnabled))
    {
        cameraComponent->SetEnabled(isEnabled);
    }

    ImGui::BeginDisabled(!isEnabled);

    ImGui::TextDisabled("View follows Transform Rotation");

    float fovYDeg = RadToDeg(cameraComponent->GetFovY());
    if (ImGui::DragFloat("FOV Y (deg)", &fovYDeg, 0.5f, 1.0f, 179.0f))
    {
        cameraComponent->SetFovY(DegToRad(fovYDeg));
    }

    float nearZ = cameraComponent->GetNearZ();
    if (ImGui::DragFloat("Near Z", &nearZ, 0.01f, 0.001f, 0.0f))
    {
        cameraComponent->SetNearZ(nearZ);
    }

    float farZ = cameraComponent->GetFarZ();
    if (ImGui::DragFloat("Far Z", &farZ, 0.5f, 0.0f, 0.0f))
    {
        cameraComponent->SetFarZ(farZ);
    }

    ImGui::Separator();

    bool isViewFrustumVisualized = cameraComponent->IsViewFrustumVisualized();
    if (ImGui::Checkbox("Visualize View Frustum", &isViewFrustumVisualized))
    {
        cameraComponent->SetViewFrustumVisualized(isViewFrustumVisualized);
    }

    ImGui::EndDisabled();

    if (ImGui::Button("Remove##Camera"))
    {
        gameObject.RemoveComponent<CameraComponent>();
    }
}

void EditorRenderer::RenderAddComponentMenu(GameObject& gameObject)
{
    if (ImGui::Button("Add Component"))
    {
        ImGui::OpenPopup(kAddComponentPopupId);
    }

    if (!ImGui::BeginPopup(kAddComponentPopupId))
    {
        return;
    }

    bool hasAnyAddable = false;
    for (const ComponentAddEntry& entry : kComponentAddEntries)
    {
        if (entry.isPresent(gameObject))
        {
            continue;
        }
        hasAnyAddable = true;
        if (ImGui::MenuItem(entry.displayName))
        {
            entry.add(gameObject);
        }
    }

    if (!hasAnyAddable)
    {
        ImGui::TextDisabled("(no more components to add)");
    }

    ImGui::EndPopup();
}

void EditorRenderer::RenderConsole()
{
    ImGui::Begin("Console");

    if (ImGui::Button("Clear"))
    {
        g_editorState->ClearConsole();
    }

    ImGui::SameLine();

    int filterLevel = static_cast<int>(g_editorState->GetConsoleFilterLevel());
    ImGui::SetNextItemWidth(100);
    if (ImGui::Combo("##Filter", &filterLevel, kLogLevelNames, IM_ARRAYSIZE(kLogLevelNames)))
    {
        g_editorState->SetConsoleFilterLevel(static_cast<LogLevel>(filterLevel));
    }

    ImGui::SameLine();

    bool autoScroll = g_editorState->GetConsoleAutoScroll();
    if (ImGui::Checkbox("AutoScroll", &autoScroll))
    {
        g_editorState->SetConsoleAutoScroll(autoScroll);
    }

    ImGui::Separator();

    ImGui::BeginChild("ConsoleScrollRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

    LogLevel currentFilter = g_editorState->GetConsoleFilterLevel();
    for (const ConsoleEntry& entry : g_editorState->GetConsoleEntries())
    {
        if (entry.level < currentFilter)
        {
            continue;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, GetLogLevelColor(entry.level));
        ImGui::TextUnformatted(entry.message.c_str());
        ImGui::PopStyleColor();
    }

    if (g_editorState->GetConsoleAutoScroll() && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputText("##ConsoleInput", g_editorState->GetConsoleInputBuffer(), kEditorConsoleInputBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        if (g_editorState->GetConsoleInputBuffer()[0] != '\0')
        {
            BA_LOG_INFO("Console> {}", g_editorState->GetConsoleInputBuffer());
            g_editorState->ClearConsoleInputBuffer();
        }

        ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::End();
}

std::unique_ptr<EditorRenderer> g_editorRenderer;

} // namespace BA
