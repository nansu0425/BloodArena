#pragma once

#include "Math/MathTypes.h"

namespace BA
{

struct Vertex
{
    Vector3 position;
    Vector3 normal;
    Vector2 uv;
};
static_assert(sizeof(Vertex) == sizeof(float) * 8);

} // namespace BA
