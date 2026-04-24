#pragma once

#include "Math/MathTypes.h"
#include "Scene/IComponent.h"

namespace BA
{

enum class LightType
{
    Directional,
    Ambient,
};

struct LightComponent : public IComponent
{
    LightType type             = LightType::Directional;
    Vector3   color            = {1.0f, 1.0f, 1.0f};
    float     intensity        = 1.0f;
    // specularStrength, shininess are Directional-only; ignored for Ambient.
    float     specularStrength = 0.5f;
    float     shininess        = 32.0f;

    LightComponent() = default;
};

} // namespace BA
