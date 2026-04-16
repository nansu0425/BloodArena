#include "Core/PCH.h"
#include "Math/Ray.h"

namespace BA
{

namespace
{

Float3 Unproject(float ndcX, float ndcY, float ndcZ, const Float4x4& invViewProj)
{
    // D3D-style NDC: z in [0, 1]. Our BuildPerspective uses this convention,
    // so the same holds for the inverse mapping.

    constexpr float kHomogeneousWEpsilon = 1e-8f;

    Float4 ndcH = { ndcX, ndcY, ndcZ, 1.0f };
    Float4 worldH = TransformHomogeneous(ndcH, invViewProj);
    BA_ASSERT(fabsf(worldH.w) > kHomogeneousWEpsilon);
    float invW = 1.0f / worldH.w;
    return { worldH.x * invW, worldH.y * invW, worldH.z * invW };
}

} // namespace

Ray BuildPickRayFromNdc(float ndcX, float ndcY, const Float4x4& view, const Float4x4& projection)
{
    Float4x4 invViewProj = Inverse(view * projection);

    Float3 nearPoint = Unproject(ndcX, ndcY, 0.0f, invViewProj);
    Float3 farPoint  = Unproject(ndcX, ndcY, 1.0f, invViewProj);

    Ray ray;
    ray.origin = nearPoint;
    ray.direction = Normalize(farPoint - nearPoint);
    return ray;
}

RayTriangleHit IntersectRayTriangle(const Ray& ray, const Float3& v0, const Float3& v1, const Float3& v2)
{
    constexpr float kUnitLengthToleranceSq = 1e-4f;
    BA_ASSERT(fabsf(LengthSquared(ray.direction) - 1.0f) < kUnitLengthToleranceSq);

    // Möller–Trumbore, two-sided.
    constexpr float kParallelEpsilon = 1e-7f;
    constexpr float kMinHitDistance = 1e-7f;

    Float3 edge1 = v1 - v0;
    Float3 edge2 = v2 - v0;

    Float3 h = Cross(ray.direction, edge2);
    float a = Dot(edge1, h);
    if (fabsf(a) < kParallelEpsilon)
    {
        return { false, 0.0f };
    }

    float f = 1.0f / a;
    Float3 s = ray.origin - v0;
    float u = f * Dot(s, h);
    if (u < 0.0f || u > 1.0f)
    {
        return { false, 0.0f };
    }

    Float3 q = Cross(s, edge1);
    float v = f * Dot(ray.direction, q);
    if (v < 0.0f || u + v > 1.0f)
    {
        return { false, 0.0f };
    }

    float t = f * Dot(edge2, q);
    if (t < kMinHitDistance)
    {
        return { false, 0.0f };
    }

    return { true, t };
}

} // namespace BA
