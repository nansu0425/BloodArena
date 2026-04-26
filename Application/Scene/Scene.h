#pragma once

#include "Scene/GameObject.h"

namespace BA
{

class Scene
{
public:
    void Initialize();
    void Shutdown();

    uint32_t CreateGameObject();
    void DestroyGameObject(uint32_t id);
    void Clear();

    std::span<const GameObject> GetGameObjects() const;
    std::span<GameObject>       GetGameObjects();
    GameObject* FindGameObject(uint32_t id);

    bool SaveToFile(const std::string& name) const;
    bool LoadFromFile(const std::string& name);

private:
    std::vector<GameObject> m_gameObjects;
    uint32_t m_nextId = 1;
};

extern std::unique_ptr<Scene> g_scene;

} // namespace BA
