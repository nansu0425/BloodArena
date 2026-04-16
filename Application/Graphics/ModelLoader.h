#pragma once

#include "Graphics/Vertex.h"

namespace BA
{

struct LoadedMeshData
{
    bool isLoaded = false;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

LoadedMeshData LoadModelFromFile(const std::string& filePath);

} // namespace BA
