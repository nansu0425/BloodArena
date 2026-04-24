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

// Arrow primitive constants (local to Gizmo.cpp).
constexpr float kArrowHeadLengthRatio = 0.15f;
constexpr float kArrowHeadRadiusRatio = 0.075f;
constexpr float kArrowLineThickness   = 2.0f;
constexpr float kUpParallelThreshold  = 0.99f;
constexpr float kNearEpsilon          = 1e-4f;

struct ScreenPoint
{
    bool   isOnScreen;
    ImVec2 screen;
};

struct ArrowBasis
{
    Vector3 right;
    Vector3 up;
};

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

ScreenPoint WorldToScreen(const Vector3& worldPos,
                          const Matrix&  viewProj,
                          const ImVec2&  rectMin,
                          const ImVec2&  rectSize)
{
    const Vector4 inPos(worldPos.x, worldPos.y, worldPos.z, 1.0f);
    const Vector4 clipPos = Vector4::Transform(inPos, viewProj);
    if (clipPos.w <= kNearEpsilon)
    {
        return {false, ImVec2(0.0f, 0.0f)};
    }
    const float ndcX = clipPos.x / clipPos.w;
    const float ndcY = clipPos.y / clipPos.w;
    const float screenX = rectMin.x + (ndcX * 0.5f + 0.5f) * rectSize.x;
    const float screenY = rectMin.y + (1.0f - (ndcY * 0.5f + 0.5f)) * rectSize.y;
    return {true, ImVec2(screenX, screenY)};
}

ArrowBasis BuildArrowBasis(const Vector3& direction)
{
    // Left-handed convention: right × up = forward, so right = up × forward.
    // When direction is near-parallel to world up, fall back to world right as
    // the reference so the first cross product is not zero.
    const float upDot = direction.Dot(kAxisUp);
    const Vector3 referenceUp = (std::abs(upDot) < kUpParallelThreshold) ? kAxisUp : kAxisRight;
    Vector3 right = referenceUp.Cross(direction);
    right.Normalize();
    Vector3 up = direction.Cross(right);
    up.Normalize();
    return {right, up};
}

void DrawLine(ImDrawList*        drawList,
              const ScreenPoint& a,
              const ScreenPoint& b,
              ImU32              color)
{
    if (!a.isOnScreen || !b.isOnScreen)
    {
        return;
    }
    drawList->AddLine(a.screen, b.screen, color, kArrowLineThickness);
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

void DrawArrow(const Vector3& origin,
               const Vector3& direction,
               float          screenSpaceLength,
               const Vector3& color,
               float          viewportX,
               float          viewportY,
               float          viewportWidth,
               float          viewportHeight,
               const Matrix&  view,
               const Matrix&  projection)
{
    const Matrix viewProj = view * projection;
    const ImVec2 rectMin(viewportX, viewportY);
    const ImVec2 rectSize(viewportWidth, viewportHeight);

    // Scale world length so the arrow projects to a constant viewport-height
    // fraction. In a standard perspective projection, screen-height of a
    // world-space segment ≈ L * P[1][1] / viewSpaceZ (in NDC), so inverting
    // gives L = screenSpaceLength * 2 * viewSpaceZ / P[1][1].
    const Vector3 viewSpaceOrigin = Vector3::Transform(origin, view);
    const float   viewSpaceZ = viewSpaceOrigin.z;
    if (viewSpaceZ <= kNearEpsilon)
    {
        return;
    }
    const float projY = projection.m[1][1];
    const float worldLength = screenSpaceLength * 2.0f * viewSpaceZ / projY;

    const ArrowBasis basis = BuildArrowBasis(direction);

    const Vector3 tip = origin + direction * worldLength;
    const float   headLength = worldLength * kArrowHeadLengthRatio;
    const float   headRadius = worldLength * kArrowHeadRadiusRatio;
    const Vector3 headBaseCenter = tip - direction * headLength;

    const Vector3 head0 = headBaseCenter + basis.right * headRadius;
    const Vector3 head1 = headBaseCenter - basis.right * headRadius;
    const Vector3 head2 = headBaseCenter + basis.up    * headRadius;
    const Vector3 head3 = headBaseCenter - basis.up    * headRadius;

    const ScreenPoint sTail = WorldToScreen(origin, viewProj, rectMin, rectSize);
    const ScreenPoint sTip  = WorldToScreen(tip,    viewProj, rectMin, rectSize);
    const ScreenPoint sH0   = WorldToScreen(head0,  viewProj, rectMin, rectSize);
    const ScreenPoint sH1   = WorldToScreen(head1,  viewProj, rectMin, rectSize);
    const ScreenPoint sH2   = WorldToScreen(head2,  viewProj, rectMin, rectSize);
    const ScreenPoint sH3   = WorldToScreen(head3,  viewProj, rectMin, rectSize);

    const ImU32 packedColor = ImGui::ColorConvertFloat4ToU32(
        ImVec4(color.x, color.y, color.z, 1.0f));
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    DrawLine(drawList, sTail, sTip, packedColor);
    DrawLine(drawList, sTip,  sH0,  packedColor);
    DrawLine(drawList, sTip,  sH1,  packedColor);
    DrawLine(drawList, sTip,  sH2,  packedColor);
    DrawLine(drawList, sTip,  sH3,  packedColor);
}

} // namespace BA::Gizmo

#endif // BA_EDITOR
