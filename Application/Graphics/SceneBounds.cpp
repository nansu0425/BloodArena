#include "Core/PCH.h"
#include "Graphics/SceneBounds.h"
#include "Graphics/Model.h"
#include "Graphics/ModelLibrary.h"
#include "Math/MathUtils.h"
#include "Scene/GameObject.h"
#include "Scene/ModelComponent.h"
#include "Scene/Scene.h"
#include <array>

namespace BA
{

namespace
{

Aabb ComputeNodeWorldAabb(
    const Model&  model,
    int           nodeIndex,
    const Matrix& parentAccumulated,
    const Matrix& objectWorld)
{
    const Node&  node        = model.nodes[nodeIndex];
    const Matrix accumulated = node.localTransform * parentAccumulated;

    Aabb result = MakeEmptyAabb();

    if (node.meshIndex >= 0)
    {
        const Matrix finalWorld = accumulated * objectWorld;
        const Mesh&  mesh       = model.meshes[node.meshIndex];

        for (const Primitive& prim : mesh.primitives)
        {
            if (!prim.localAabb.isValid)
            {
                continue;
            }

            const std::array<Vector3, kAabbCornerCount> corners =
                GetAabbCorners(prim.localAabb);
            for (const Vector3& localCorner : corners)
            {
                const Vector3 worldCorner = Vector3::Transform(localCorner, finalWorld);
                result = ExpandAabbWithPoint(result, worldCorner);
            }
        }
    }

    for (int childIndex : node.childIndices)
    {
        const Aabb childAabb =
            ComputeNodeWorldAabb(model, childIndex, accumulated, objectWorld);
        result = MergeAabb(result, childAabb);
    }

    return result;
}

} // namespace

Aabb ComputeSceneWorldAabb(const Scene& scene)
{
    BA_PROFILE_SCOPE("ComputeSceneWorldAabb");

    Aabb result = MakeEmptyAabb();

    for (const GameObject& obj : scene.GetGameObjects())
    {
        const ModelComponent* mc = obj.GetComponent<ModelComponent>();
        if (!mc || !mc->IsEnabled())
        {
            continue;
        }
        const Model* model = g_modelLibrary->FindModel(mc->GetModelName());
        if (!model)
        {
            continue;
        }

        const Matrix objectWorld = BuildWorld(obj.GetTransform());
        for (int rootIndex : model->rootNodeIndices)
        {
            const Aabb objectAabb =
                ComputeNodeWorldAabb(*model, rootIndex, Matrix::Identity, objectWorld);
            result = MergeAabb(result, objectAabb);
        }
    }

    return result;
}

} // namespace BA
