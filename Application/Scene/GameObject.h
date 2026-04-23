#pragma once

#include "Math/MathUtils.h"
#include "Scene/ModelComponent.h"

namespace BA
{

struct GameObject
{
    uint32_t id = 0;
    Transform transform;
    std::unique_ptr<ModelComponent> modelComponent;
};

} // namespace BA
