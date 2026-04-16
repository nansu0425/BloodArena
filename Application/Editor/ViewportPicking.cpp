#include "Core/PCH.h"
#include <cfloat>
#include "Editor/ViewportPicking.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"
#include "Math/MathUtils.h"

namespace BA
{

namespace
{

const Vector3 kTriangleVertices[3] =
{
    { 0.0f,   0.06f, 0.0f},
    { 0.06f, -0.04f, 0.0f},
    {-0.06f, -0.04f, 0.0f},
};

Ray BuildPickRayFromNdc(float ndcX, float ndcY, const Matrix& view, const Matrix& projection)
{
    Matrix invViewProj = (view * projection).Invert();

    Vector3 nearPoint = Vector3::Transform(Vector3(ndcX, ndcY, 0.0f), invViewProj);
    Vector3 farPoint  = Vector3::Transform(Vector3(ndcX, ndcY, 1.0f), invViewProj);

    Vector3 direction = farPoint - nearPoint;
    direction.Normalize();

    return Ray(nearPoint, direction);
}

} // namespace

uint32_t PickGameObject(float ndcX, float ndcY, const Camera& camera, float aspect)
{
    Ray ray = BuildPickRayFromNdc(ndcX, ndcY, camera.GetViewMatrix(), camera.GetProjectionMatrix(aspect));

    uint32_t hitId = 0;
    float closestT = FLT_MAX;

    for (const GameObject& gameObject : g_scene->GetGameObjects())
    {
        Matrix worldMatrix = BuildWorld(gameObject.transform);

        Vector3 v0 = Vector3::Transform(kTriangleVertices[0], worldMatrix);
        Vector3 v1 = Vector3::Transform(kTriangleVertices[1], worldMatrix);
        Vector3 v2 = Vector3::Transform(kTriangleVertices[2], worldMatrix);

        float distance = 0.0f;
        if (ray.Intersects(v0, v1, v2, distance) && distance < closestT)
        {
            closestT = distance;
            hitId = gameObject.id;
        }
    }

    return hitId;
}

} // namespace BA
