#include "Core/PCH.h"
#include <cfloat>
#include "Editor/ViewportPicking.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"
#include "Graphics/ModelLibrary.h"
#include "Math/MathUtils.h"

namespace BA
{

namespace
{

struct PickResult
{
    bool isHit = false;
    uint32_t hitId = 0;
    float closestT = FLT_MAX;
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

PickResult PickNode(
    const Ray& ray,
    const Model& model,
    int nodeIndex,
    const Matrix& parentAccumulated,
    const Matrix& objectWorld,
    uint32_t gameObjectId)
{
    // Row-vector convention, mirroring SceneRenderer::DrawNode: v' = v * (local * parent) * objectWorld.
    const Node& node = model.nodes[nodeIndex];
    Matrix accumulated = node.localTransform * parentAccumulated;

    PickResult best;

    if (node.meshIndex >= 0)
    {
        constexpr int kIndicesPerTriangle = 3;

        Matrix finalWorld = accumulated * objectWorld;
        const Mesh& mesh = model.meshes[node.meshIndex];

        for (const Primitive& prim : mesh.primitives)
        {
            const size_t triangleCount = prim.pickingIndices.size() / kIndicesPerTriangle;
            for (size_t t = 0; t < triangleCount; ++t)
            {
                const uint32_t i0 = prim.pickingIndices[t * kIndicesPerTriangle + 0];
                const uint32_t i1 = prim.pickingIndices[t * kIndicesPerTriangle + 1];
                const uint32_t i2 = prim.pickingIndices[t * kIndicesPerTriangle + 2];

                Vector3 v0 = Vector3::Transform(prim.pickingPositions[i0], finalWorld);
                Vector3 v1 = Vector3::Transform(prim.pickingPositions[i1], finalWorld);
                Vector3 v2 = Vector3::Transform(prim.pickingPositions[i2], finalWorld);

                float distance = 0.0f;
                if (ray.Intersects(v0, v1, v2, distance) && distance < best.closestT)
                {
                    best.isHit = true;
                    best.hitId = gameObjectId;
                    best.closestT = distance;
                }
            }
        }
    }

    for (int childIndex : node.childIndices)
    {
        PickResult child = PickNode(ray, model, childIndex, accumulated, objectWorld, gameObjectId);
        if (child.isHit && child.closestT < best.closestT)
        {
            best = child;
        }
    }

    return best;
}

} // namespace

uint32_t PickGameObject(float ndcX, float ndcY, const Camera& camera, float aspect)
{
    Ray ray = BuildPickRayFromNdc(ndcX, ndcY, camera.GetViewMatrix(), camera.GetProjectionMatrix(aspect));

    uint32_t hitId = 0;
    float closestT = FLT_MAX;

    for (const GameObject& gameObject : g_scene->GetGameObjects())
    {
        if (!gameObject.modelComponent)
        {
            continue;
        }
        const Model* model = g_modelLibrary->FindModel(gameObject.modelComponent->modelName);
        BA_ASSERT(model);

        Matrix objectWorld = BuildWorld(gameObject.transform);
        for (int rootIndex : model->rootNodeIndices)
        {
            PickResult result = PickNode(ray, *model, rootIndex, Matrix::Identity, objectWorld, gameObject.id);
            if (result.isHit && result.closestT < closestT)
            {
                closestT = result.closestT;
                hitId = result.hitId;
            }
        }
    }

    return hitId;
}

} // namespace BA
