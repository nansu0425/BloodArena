#include "Core/PCH.h"
#include <cfloat>
#include "Editor/ViewportPicking.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"
#include "Math/Transform.h"
#include "Math/Ray.h"

namespace BA
{

namespace
{

const Float3 kTriangleVertices[3] =
{
    { 0.0f,   0.06f, 0.0f},
    { 0.06f, -0.04f, 0.0f},
    {-0.06f, -0.04f, 0.0f},
};

} // namespace

uint32_t PickGameObject(float ndcX, float ndcY, const Camera& camera, float aspect)
{
    Ray ray = BuildPickRayFromNdc(ndcX, ndcY, camera.GetViewMatrix(), camera.GetProjectionMatrix(aspect));

    uint32_t hitId = 0;
    float closestT = FLT_MAX;

    for (const GameObject& gameObject : g_scene->GetGameObjects())
    {
        Float4x4 worldMatrix = BuildWorld(gameObject.transform);

        Float3 v0 = TransformPoint(kTriangleVertices[0], worldMatrix);
        Float3 v1 = TransformPoint(kTriangleVertices[1], worldMatrix);
        Float3 v2 = TransformPoint(kTriangleVertices[2], worldMatrix);

        RayTriangleHit hit = IntersectRayTriangle(ray, v0, v1, v2);
        if (hit.isHit && hit.t < closestT)
        {
            closestT = hit.t;
            hitId = gameObject.id;
        }
    }

    return hitId;
}

} // namespace BA
