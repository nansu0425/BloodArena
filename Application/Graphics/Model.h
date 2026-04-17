#pragma once

#include "Math/MathTypes.h"

namespace BA
{

struct Material
{
    std::string diffuseTextureName;
};

struct Primitive
{
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    uint32_t indexCount = 0;
    bool isIndex32Bit = false;
    int materialIndex = -1;
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
