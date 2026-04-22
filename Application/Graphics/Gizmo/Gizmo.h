#pragma once
#if defined(BA_EDITOR)

#include "Math/MathTypes.h"
#include "Math/MathUtils.h"

namespace BA::Gizmo
{

enum class Mode
{
    None,
    Translate,
    Rotate,
    Scale,
};

enum class Space
{
    World,
    Local,
};

struct ManipulateResult
{
    bool      isChanged;
    Transform transform;
};

void Initialize();
void Shutdown();

// Call once right after ImGui::NewFrame() and before any ImGui window.
void BeginFrame();

// Call while the viewport ImGui window is current (before ImGui::Image),
// with the viewport rect in screen coords from ImGui::GetItemRectMin/Max.
void SetViewportRect(float screenX, float screenY, float width, float height);

// No-op returning {false, in} when mode == None. Otherwise draws the gizmo
// and, if the user is manipulating it, returns {true, modifiedTransform}.
ManipulateResult Manipulate(const Transform& in,
                            Mode              mode,
                            Space             space,
                            const Matrix&     view,
                            const Matrix&     projection);

// True while the mouse is over an active gizmo handle or dragging one.
// Used to suppress the existing PickGameObject click-to-select flow.
bool IsUsingMouse();

} // namespace BA::Gizmo

#endif // BA_EDITOR
