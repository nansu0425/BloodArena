#pragma once

#include "Math/Float3.h"

namespace BA
{

struct Transform
{
    Float3 position = {0.0f, 0.0f, 0.0f};
    Float3 rotation = {0.0f, 0.0f, 0.0f};
    Float3 scale    = {1.0f, 1.0f, 1.0f};
};

} // namespace BA
