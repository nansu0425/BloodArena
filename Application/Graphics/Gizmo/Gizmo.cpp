#include "Core/PCH.h"
#if defined(BA_EDITOR)

#include "Graphics/Gizmo/Gizmo.h"

#pragma warning(push, 0)
#include <imgui.h>
#include <ImGuizmo.h>
#pragma warning(pop)

namespace BA::Gizmo
{

namespace
{

ImGuizmo::OPERATION ToImGuizmoOperation(Mode mode)
{
    switch (mode)
    {
    case Mode::Translate: return ImGuizmo::TRANSLATE;
    case Mode::Rotate:    return ImGuizmo::ROTATE;
    case Mode::Scale:     return ImGuizmo::SCALE;
    default:              return ImGuizmo::TRANSLATE;
    }
}

ImGuizmo::MODE ToImGuizmoMode(Space space)
{
    switch (space)
    {
    case Space::World: return ImGuizmo::WORLD;
    case Space::Local: return ImGuizmo::LOCAL;
    default:           return ImGuizmo::WORLD;
    }
}

} // namespace

void Initialize()
{
    BA_LOG_INFO("Gizmo initialized.");
}

void Shutdown()
{
    BA_LOG_INFO("Gizmo shutdown.");
}

void BeginFrame()
{
    ImGuizmo::BeginFrame();
}

void SetViewportRect(float screenX, float screenY, float width, float height)
{
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(screenX, screenY, width, height);
}

ManipulateResult Manipulate(const Transform& in,
                            Mode              mode,
                            Space             space,
                            const Matrix&     view,
                            const Matrix&     projection)
{
    if (mode == Mode::None)
    {
        return {false, in};
    }

    Matrix world = BuildWorld(in);

    ImGuizmo::Manipulate(
        &view.m[0][0],
        &projection.m[0][0],
        ToImGuizmoOperation(mode),
        ToImGuizmoMode(space),
        &world.m[0][0]
    );

    if (!ImGuizmo::IsUsing())
    {
        return {false, in};
    }

    Vector3    resultScale;
    Quaternion resultRotation;
    Vector3    resultPosition;
    world.Decompose(resultScale, resultRotation, resultPosition);

    Transform result;
    result.position = resultPosition;
    result.rotation = resultRotation;
    result.scale    = resultScale;

    return {true, result};
}

bool IsUsingMouse()
{
    return ImGuizmo::IsUsing() || ImGuizmo::IsOver();
}

} // namespace BA::Gizmo

#endif // BA_EDITOR
