#include "Core/PCH.h"
#include "Editor/ViewportPicking.h"
#include "Scene/Scene.h"
#include "Math/MathUtils.h"

namespace BA
{

static constexpr float kTriangleVertices[3][3] =
{
    { 0.0f,   0.06f, 0.0f},
    { 0.06f, -0.04f, 0.0f},
    {-0.06f, -0.04f, 0.0f},
};

uint32_t PickGameObject(float ndcX, float ndcY)
{
    auto gameObjects = g_scene->GetGameObjects();

    for (int32_t i = static_cast<int32_t>(gameObjects.size()) - 1; i >= 0; --i)
    {
        const GameObject& gameObject = gameObjects[i];

        Matrix4x4 worldMatrix = BuildWorldMatrix(gameObject.transform);

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
