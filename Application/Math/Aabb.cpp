#include "Core/PCH.h"
#include "Math/Aabb.h"
#include <algorithm>
#include <cfloat>

namespace BA
{

Aabb MakeEmptyAabb()
{
    Aabb aabb;
    aabb.isValid   = false;
    aabb.minCorner = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
    aabb.maxCorner = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    return aabb;
}

Aabb MergeAabb(const Aabb& a, const Aabb& b)
{
    if (!a.isValid)
    {
        return b;
    }

    if (!b.isValid)
    {
        return a;
    }

    Aabb merged;
    merged.isValid     = true;
    merged.minCorner.x = std::min(a.minCorner.x, b.minCorner.x);
    merged.minCorner.y = std::min(a.minCorner.y, b.minCorner.y);
    merged.minCorner.z = std::min(a.minCorner.z, b.minCorner.z);
    merged.maxCorner.x = std::max(a.maxCorner.x, b.maxCorner.x);
    merged.maxCorner.y = std::max(a.maxCorner.y, b.maxCorner.y);
    merged.maxCorner.z = std::max(a.maxCorner.z, b.maxCorner.z);

    return merged;
}

Aabb ExpandAabbWithPoint(const Aabb& aabb, const Vector3& point)
{
    Aabb expanded;
    expanded.isValid = true;

    if (!aabb.isValid)
    {
        expanded.minCorner = point;
        expanded.maxCorner = point;

        return expanded;
    }

    expanded.minCorner.x = std::min(aabb.minCorner.x, point.x);
    expanded.minCorner.y = std::min(aabb.minCorner.y, point.y);
    expanded.minCorner.z = std::min(aabb.minCorner.z, point.z);
    expanded.maxCorner.x = std::max(aabb.maxCorner.x, point.x);
    expanded.maxCorner.y = std::max(aabb.maxCorner.y, point.y);
    expanded.maxCorner.z = std::max(aabb.maxCorner.z, point.z);

    return expanded;
}

Aabb IntersectAabb(const Aabb& a, const Aabb& b)
{
    Aabb result;
    result.isValid   = false;
    result.minCorner = Vector3(0.0f, 0.0f, 0.0f);
    result.maxCorner = Vector3(0.0f, 0.0f, 0.0f);

    if (!a.isValid || !b.isValid)
    {
        return result;
    }

    const Vector3 lo = Vector3(
        std::max(a.minCorner.x, b.minCorner.x),
        std::max(a.minCorner.y, b.minCorner.y),
        std::max(a.minCorner.z, b.minCorner.z));
    const Vector3 hi = Vector3(
        std::min(a.maxCorner.x, b.maxCorner.x),
        std::min(a.maxCorner.y, b.maxCorner.y),
        std::min(a.maxCorner.z, b.maxCorner.z));
    if (lo.x > hi.x || lo.y > hi.y || lo.z > hi.z)
    {
        return result;
    }

    result.isValid   = true;
    result.minCorner = lo;
    result.maxCorner = hi;

    return result;
}

std::array<Vector3, kAabbCornerCount> GetAabbCorners(const Aabb& aabb)
{
    const Vector3& lo = aabb.minCorner;
    const Vector3& hi = aabb.maxCorner;

    return std::array<Vector3, kAabbCornerCount>{
        Vector3(lo.x, lo.y, lo.z),
        Vector3(hi.x, lo.y, lo.z),
        Vector3(lo.x, hi.y, lo.z),
        Vector3(hi.x, hi.y, lo.z),
        Vector3(lo.x, lo.y, hi.z),
        Vector3(hi.x, lo.y, hi.z),
        Vector3(lo.x, hi.y, hi.z),
        Vector3(hi.x, hi.y, hi.z),
    };
}

} // namespace BA
