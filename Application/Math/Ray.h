#pragma once

#include "Math/Float4x4.h"

namespace BA
{

struct Ray
{
    Float3 origin;
    Float3 direction;  // Must be a unit vector.
};

struct RayTriangleHit
{
    bool isHit;
    float t;
};

Ray BuildPickRayFromNdc(float ndcX, float ndcY, const Float4x4& view, const Float4x4& projection);

RayTriangleHit IntersectRayTriangle(const Ray& ray, const Float3& v0, const Float3& v1, const Float3& v2);

} // namespace BA
