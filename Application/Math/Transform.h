#pragma once

#include "Math/Float3.h"
#include "Math/Float4x4.h"

namespace BA
{

struct Transform
{
    Float3 position = {0.0f, 0.0f, 0.0f};
    Float3 rotation = {0.0f, 0.0f, 0.0f};
    Float3 scale    = {1.0f, 1.0f, 1.0f};
};

Float4x4 BuildWorld(const Transform& transform);

} // namespace BA
