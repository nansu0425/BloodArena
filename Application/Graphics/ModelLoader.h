#pragma once

#include "Graphics/Vertex.h"

namespace BA
{

struct LoadedMeshData
{
    bool isLoaded = false;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    bool hasTexture = false;
    std::vector<uint8_t> textureRgba8;
    uint32_t textureWidth = 0;
    uint32_t textureHeight = 0;
};

LoadedMeshData LoadModelFromFile(const std::string& filePath);

} // namespace BA
