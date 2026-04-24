#pragma once

#include "Math/MathTypes.h"
#include "Scene/IComponent.h"

namespace BA
{

enum class LightType
{
    Directional,
};

struct LightComponent : public IComponent
{
    LightType type             = LightType::Directional;
    Vector3   color            = {1.0f, 1.0f, 1.0f};
    float     intensity        = 1.0f;
    Vector3   ambientColor     = {0.15f, 0.15f, 0.18f};
    float     specularStrength = 0.5f;
    float     shininess        = 32.0f;

    LightComponent() = default;
};

} // namespace BA
