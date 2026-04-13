#include "Core/PCH.h"
#include "Editor/ViewportPicking.h"
#include "Scene/Scene.h"
#include "Math/Transform.h"

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

bool IsPointInTriangle(float px, float py, float ax, float ay, float bx, float by, float cx, float cy)
{
    float d1 = (bx - ax) * (py - ay) - (by - ay) * (px - ax);
    float d2 = (cx - bx) * (py - by) - (cy - by) * (px - bx);
    float d3 = (ax - cx) * (py - cy) - (ay - cy) * (px - cx);

    bool hasNeg = (d1 < 0.0f) || (d2 < 0.0f) || (d3 < 0.0f);
    bool hasPos = (d1 > 0.0f) || (d2 > 0.0f) || (d3 > 0.0f);

    return !(hasNeg && hasPos);
}

} // namespace

uint32_t PickGameObject(float ndcX, float ndcY)
{
    auto gameObjects = g_scene->GetGameObjects();

    for (int32_t i = static_cast<int32_t>(gameObjects.size()) - 1; i >= 0; --i)
    {
        const GameObject& gameObject = gameObjects[i];

        Float4x4 worldMatrix = BuildWorld(gameObject.transform);

        Float3 v0 = TransformPoint(kTriangleVertices[0], worldMatrix);
        Float3 v1 = TransformPoint(kTriangleVertices[1], worldMatrix);
        Float3 v2 = TransformPoint(kTriangleVertices[2], worldMatrix);

        if (IsPointInTriangle(ndcX, ndcY, v0.x, v0.y, v1.x, v1.y, v2.x, v2.y))
        {
            return gameObject.id;
        }
    }

    return 0;
}

} // namespace BA
