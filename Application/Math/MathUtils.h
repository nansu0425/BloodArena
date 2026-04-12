#pragma once

#include "Math/Float3.h"
#include "Math/Float4x4.h"

namespace BA
{

struct Transform;

Float4x4 BuildWorld(const Transform& transform);
bool IsPointInTriangle(float px, float py, float ax, float ay, float bx, float by, float cx, float cy);

} // namespace BA
