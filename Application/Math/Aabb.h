#pragma once

#include "Math/MathTypes.h"

namespace BA
{

struct Aabb
{
    bool    isValid;
    Vector3 minCorner;
    Vector3 maxCorner;
};

Aabb MakeEmptyAabb();
Aabb MergeAabb(const Aabb& a, const Aabb& b);
Aabb ExpandAabbWithPoint(const Aabb& aabb, const Vector3& point);

} // namespace BA
