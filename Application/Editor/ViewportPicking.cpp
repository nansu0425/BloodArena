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

constexpr int kTrianglesPerFace = 2;
constexpr int kVerticesPerFace = 4;
constexpr int kFaceCount = 6;
constexpr int kCubeTriangleCount = kFaceCount * kTrianglesPerFace;

const Vector3 kCubePositions[kFaceCount * kVerticesPerFace] =
{
    // +Z
    {-0.5f,  0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f}, {-0.5f, -0.5f,  0.5f},
    // -Z
    { 0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f},
    // +Y
    {-0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f},
    // -Y
    {-0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f},
    // +X
    { 0.5f,  0.5f,  0.5f}, { 0.5f,  0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f,  0.5f},
    // -X
    {-0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f,  0.5f}, {-0.5f, -0.5f,  0.5f}, {-0.5f, -0.5f, -0.5f},
};

const uint16_t kCubeIndices[kCubeTriangleCount * 3] =
{
     0,  2,  1,   0,  3,  2,
     4,  6,  5,   4,  7,  6,
     8, 10,  9,   8, 11, 10,
    12, 14, 13,  12, 15, 14,
    16, 18, 17,  16, 19, 18,
    20, 22, 21,  20, 23, 22,
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

        for (int i = 0; i < kCubeTriangleCount; ++i)
        {
            Vector3 v0 = Vector3::Transform(kCubePositions[kCubeIndices[i * 3 + 0]], worldMatrix);
            Vector3 v1 = Vector3::Transform(kCubePositions[kCubeIndices[i * 3 + 1]], worldMatrix);
            Vector3 v2 = Vector3::Transform(kCubePositions[kCubeIndices[i * 3 + 2]], worldMatrix);

            float distance = 0.0f;
            if (ray.Intersects(v0, v1, v2, distance) && distance < closestT)
            {
                closestT = distance;
                hitId = gameObject.id;
            }
        }
    }

    return hitId;
}

} // namespace BA
