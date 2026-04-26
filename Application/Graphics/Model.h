#pragma once

#include "Math/Aabb.h"
#include "Math/MathTypes.h"

namespace BA
{

enum class AlphaMode : uint32_t
{
    Opaque = 0,
    Mask   = 1,
};

inline constexpr float kDefaultAlphaCutoff = 0.5f;

struct Material
{
    std::string diffuseTextureName;
    float       baseColorFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    AlphaMode   alphaMode          = AlphaMode::Opaque;
    float       alphaCutoff        = kDefaultAlphaCutoff;
    bool        isDoubleSided      = false;
};

struct Primitive
{
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    uint32_t indexCount = 0;
    bool isIndex32Bit = false;
    int materialIndex = -1;

    std::vector<Vector3> cpuPositions;
    std::vector<uint32_t> cpuIndices;

    Aabb localAabb = MakeEmptyAabb();
};

struct Mesh
{
    std::vector<Primitive> primitives;
};

struct Node
{
    Matrix localTransform = Matrix::Identity;
    int meshIndex = -1;
    std::vector<int> childIndices;
};

struct Model
{
    std::vector<Node> nodes;
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::vector<int> rootNodeIndices;
};

} // namespace BA
