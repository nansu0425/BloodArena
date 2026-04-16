#include "Core/PCH.h"
#include "Graphics/EditorRenderer.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/SceneRenderer.h"
#include "Graphics/SceneViewport.h"
#include "Editor/EditorUI.h"
#include "Editor/ViewportPicking.h"
#include "Core/Window.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"
#include "Math/Angle.h"

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

void EditorRenderer::Render()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_None);

    RenderViewport();
    RenderHierarchy();
    RenderInspector();
    RenderConsole();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void EditorRenderer::RenderViewport()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_MenuBar);
    ImGui::PopStyleVar();

    if (ImGui::BeginMenuBar())
    {
        const char* kCameraLabel = "Camera";
        float buttonWidth = ImGui::CalcTextSize(kCameraLabel).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float available = ImGui::GetContentRegionAvail().x;
        if (available > buttonWidth)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available - buttonWidth);
        }
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

        g_sceneViewport->Clear();

        float aspect = size.x / size.y;
        g_sceneRenderer->Render(aspect);

        g_graphicsDevice->RestoreBackBuffer();

        ImGui::Image(
            reinterpret_cast<ImTextureID>(g_sceneViewport->GetSRV()),
            size
        );

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            ImVec2 imageMin = ImGui::GetItemRectMin();
            ImVec2 mousePos = ImGui::GetMousePos();

            float pixelX = mousePos.x - imageMin.x;
            float pixelY = mousePos.y - imageMin.y;

            float ndcX = (pixelX / size.x) * 2.0f - 1.0f;
            float ndcY = 1.0f - (pixelY / size.y) * 2.0f;

            uint32_t hitId = PickGameObject(ndcX, ndcY, *g_camera, aspect);
            g_editorUI->SetSelectedGameObjectId(hitId);
        }
    }

    ImGui::End();
}

void EditorRenderer::RenderCameraSettingsMenu()
{
    BA_ASSERT(g_camera);

    CameraSettings settings = g_camera->GetSettings();

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

    g_camera->SetSettings(settings);

    ImGui::Separator();
    if (ImGui::Button("Reset"))
    {
        g_camera->ResetToDefaults();
    }
}

void EditorRenderer::RenderHierarchy()
{
    ImGui::Begin("Hierarchy");

    uint32_t selectedId = g_editorUI->GetSelectedGameObjectId();

    for (const GameObject& gameObject : g_scene->GetGameObjects())
    {
        bool isSelected = (selectedId == gameObject.id);

        char label[32];
        snprintf(label, sizeof(label), "GameObject %u", gameObject.id);

        if (ImGui::Selectable(label, isSelected))
        {
            g_editorUI->SetSelectedGameObjectId(gameObject.id);
        }
    }

    ImGui::End();
}

void EditorRenderer::RenderInspector()
{
    ImGui::Begin("Inspector");

    uint32_t selectedId = g_editorUI->GetSelectedGameObjectId();

    if (selectedId == 0)
    {
        ImGui::TextDisabled("No object selected");
        ImGui::End();
        return;
    }

    GameObject* selected = g_scene->FindGameObject(selectedId);
    BA_ASSERT(selected != nullptr);

    ImGui::Text("ID: %u", selected->id);
    ImGui::Separator();

    ImGui::DragFloat3("Position", &selected->transform.position.x, 0.01f);
    ImGui::DragFloat3("Rotation", &selected->transform.rotation.x, 0.1f);
    ImGui::DragFloat3("Scale", &selected->transform.scale.x, 0.01f);

    ImGui::Separator();

    ImGui::ColorEdit4("Color", selected->color);

    ImGui::End();
}

void EditorRenderer::RenderConsole()
{
    ImGui::Begin("Console");

    if (ImGui::Button("Clear"))
    {
        g_editorUI->ClearConsole();
    }

    ImGui::SameLine();

    int filterLevel = static_cast<int>(g_editorUI->GetConsoleFilterLevel());
    ImGui::SetNextItemWidth(100);
    if (ImGui::Combo("##Filter", &filterLevel, kLogLevelNames, IM_ARRAYSIZE(kLogLevelNames)))
    {
        g_editorUI->SetConsoleFilterLevel(static_cast<LogLevel>(filterLevel));
    }

    ImGui::SameLine();

    bool autoScroll = g_editorUI->GetConsoleAutoScroll();
    if (ImGui::Checkbox("AutoScroll", &autoScroll))
    {
        g_editorUI->SetConsoleAutoScroll(autoScroll);
    }

    ImGui::Separator();

    ImGui::BeginChild("ConsoleScrollRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

    LogLevel currentFilter = g_editorUI->GetConsoleFilterLevel();
    for (const ConsoleEntry& entry : g_editorUI->GetConsoleEntries())
    {
        if (entry.level < currentFilter)
        {
            continue;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, GetLogLevelColor(entry.level));
        ImGui::TextUnformatted(entry.message.c_str());
        ImGui::PopStyleColor();
    }

    if (g_editorUI->GetConsoleAutoScroll() && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputText("##ConsoleInput", m_consoleInputBuffer, sizeof(m_consoleInputBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        if (m_consoleInputBuffer[0] != '\0')
        {
            BA_LOG_INFO("Console> {}", m_consoleInputBuffer);
            m_consoleInputBuffer[0] = '\0';
        }

        ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::End();
}

std::unique_ptr<EditorRenderer> g_editorRenderer;

} // namespace BA
