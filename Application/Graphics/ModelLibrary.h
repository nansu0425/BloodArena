#pragma once

#include "Graphics/Model.h"

namespace BA
{

constexpr const char* kDefaultModelName = "cube";

class ModelLibrary
{
public:
    void Initialize();
    void Shutdown();

    const Model* FindModel(const std::string& name) const;
    bool LoadModel(const std::string& name, const std::string& filePath);
    std::vector<std::string> GetModelNames() const;

private:
    void CreateBuiltInCube();
    void LoadModelsFromAssetsDirectory();

    std::unordered_map<std::string, Model> m_models;
};

extern std::unique_ptr<ModelLibrary> g_modelLibrary;

} // namespace BA
