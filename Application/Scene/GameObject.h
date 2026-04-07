#pragma once

#include "Math/Transform.h"

namespace BA
{

struct GameObject
{
    uint32_t id = 0;
    float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    Transform transform;
};

} // namespace BA
