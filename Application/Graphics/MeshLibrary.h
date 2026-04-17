#pragma once

#include "Graphics/Mesh.h"

namespace BA
{

class MeshLibrary
{
public:
    void Initialize();
    void Shutdown();

    const Mesh* FindMesh(const std::string& name) const;
    const Mesh* GetDefaultMesh() const;
    bool LoadMesh(const std::string& name, const std::string& filePath);
    std::vector<std::string> GetMeshNames() const;

private:
    void CreateBuiltInCube();
    void LoadModelsFromAssetsDirectory();

    std::unordered_map<std::string, Mesh> m_meshes;
};

extern std::unique_ptr<MeshLibrary> g_meshLibrary;

} // namespace BA
