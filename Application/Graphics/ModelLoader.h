#pragma once

#include "Graphics/Vertex.h"

namespace BA
{

struct LoadedPrimitiveData
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    int materialIndex = -1;
};

struct LoadedMeshData
{
    std::vector<LoadedPrimitiveData> primitives;
};

struct LoadedMaterialData
{
    bool hasDiffuseTexture = false;
    std::vector<uint8_t> diffuseRgba8;
    uint32_t diffuseWidth = 0;
    uint32_t diffuseHeight = 0;
};

struct LoadedNodeData
{
    Matrix localTransform = Matrix::Identity;
    int meshIndex = -1;
    std::vector<int> childIndices;
};

struct LoadedModelData
{
    bool isLoaded = false;
    std::vector<LoadedNodeData> nodes;
    std::vector<LoadedMeshData> meshes;
    std::vector<LoadedMaterialData> materials;
    std::vector<int> rootNodeIndices;
};

LoadedModelData LoadModelFromFile(const std::string& filePath);

} // namespace BA
