#include "Core/PCH.h"
#include "Graphics/EditorRenderer.h"
#include "Editor/EditorUI.h"
#include "Core/Window.h"
#include "Graphics/GraphicsDevice.h"
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

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    RenderHierarchy();
    RenderInspector();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
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

    const GameObject* selected = nullptr;
    for (const GameObject& gameObject : g_scene->GetGameObjects())
    {
        if (gameObject.id == selectedId)
        {
            selected = &gameObject;
            break;
        }
    }

    if (selected == nullptr)
    {
        g_editorUI->SetSelectedGameObjectId(0);
        ImGui::TextDisabled("No object selected");
        ImGui::End();
        return;
    }

    ImGui::Text("ID: %u", selected->id);
    ImGui::Separator();

    ImGui::Text("Position: %.3f, %.3f, %.3f",
        selected->transform.position[0],
        selected->transform.position[1],
        selected->transform.position[2]);

    ImGui::Text("Rotation: %.3f", selected->transform.rotation);

    ImGui::Text("Scale: %.3f, %.3f, %.3f",
        selected->transform.scale[0],
        selected->transform.scale[1],
        selected->transform.scale[2]);

    ImGui::Separator();

    ImGui::ColorButton("##color", ImVec4(
        selected->color[0],
        selected->color[1],
        selected->color[2],
        selected->color[3]));
    ImGui::SameLine();
    ImGui::Text("Color: %.2f, %.2f, %.2f, %.2f",
        selected->color[0],
        selected->color[1],
        selected->color[2],
        selected->color[3]);

    ImGui::End();
}

std::unique_ptr<EditorRenderer> g_editorRenderer;

} // namespace BA
