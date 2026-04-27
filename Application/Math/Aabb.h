#pragma once

#include "Math/MathTypes.h"
#include <array>

namespace BA
{

inline constexpr size_t kAabbCornerCount = 8;

struct Aabb
{
    bool    isValid;
    Vector3 minCorner;
    Vector3 maxCorner;
};

Aabb MakeEmptyAabb();
Aabb MergeAabb(const Aabb& a, const Aabb& b);
Aabb ExpandAabbWithPoint(const Aabb& aabb, const Vector3& point);
Aabb IntersectAabb(const Aabb& a, const Aabb& b);

std::array<Vector3, kAabbCornerCount> GetAabbCorners(const Aabb& aabb);

} // namespace BA
