#include "Core/PCH.h"
#include "Graphics/EditorRenderer.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/SceneRenderer.h"
#include "Graphics/SceneViewport.h"
#include "Editor/EditorUI.h"
#include "Editor/ViewportPicking.h"
#include "Core/Window.h"
#include "Scene/Scene.h"

#pragma warning(push, 0)
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#pragma warning(pop)

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace BA
{

static LRESULT ImGuiWndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
}

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
    ImGui::Begin("Viewport");
    ImGui::PopStyleVar();

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
        g_sceneRenderer->Render();

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

            uint32_t hitId = PickGameObject(ndcX, ndcY);
            g_editorUI->SetSelectedGameObjectId(hitId);
        }
    }

    ImGui::End();
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

    ImGui::DragFloat3("Position", selected->transform.position, 0.01f);
    ImGui::DragFloat("Rotation", &selected->transform.rotation, 0.1f);
    ImGui::DragFloat3("Scale", selected->transform.scale, 0.01f);

    ImGui::Separator();

    ImGui::ColorEdit4("Color", selected->color);

    ImGui::End();
}

static ImVec4 GetLogLevelColor(LogLevel level)
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

static const char* kLogLevelNames[] = { "Trace", "Debug", "Info", "Warn", "Error", "Critical" };

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
