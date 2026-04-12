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

    std::span<const GameObject> GetGameObjects() const;
    GameObject* FindGameObject(uint32_t id);

private:
    std::vector<GameObject> m_gameObjects;
    uint32_t m_nextId = 1;
};

extern std::unique_ptr<Scene> g_scene;

} // namespace BA
