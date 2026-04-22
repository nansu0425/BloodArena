#include "Core/PCH.h"
#if defined(BA_EDITOR)

#include "Graphics/Gizmo/Gizmo.h"

namespace BA::Gizmo
{

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
}

void SetViewportRect(float, float, float, float)
{
}

ManipulateResult Manipulate(const Transform& in,
                            Mode,
                            Space,
                            const Matrix&,
                            const Matrix&)
{
    return {false, in};
}

bool IsUsingMouse()
{
    return false;
}

} // namespace BA::Gizmo

#endif // BA_EDITOR
