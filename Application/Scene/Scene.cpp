#include "Core/PCH.h"
#include "Scene/Scene.h"

namespace BA
{

constexpr float kPalette[][4] =
{
    {1.0f, 0.2f, 0.2f, 1.0f},
    {0.2f, 1.0f, 0.2f, 1.0f},
    {0.2f, 0.4f, 1.0f, 1.0f},
    {1.0f, 1.0f, 0.2f, 1.0f},
    {1.0f, 0.2f, 1.0f, 1.0f},
    {0.2f, 1.0f, 1.0f, 1.0f},
    {1.0f, 0.6f, 0.2f, 1.0f},
    {0.6f, 0.2f, 1.0f, 1.0f},
};

constexpr uint32_t kPaletteSize = _countof(kPalette);
constexpr uint32_t kGridColumns = 5;
constexpr float kGridStartX = -4.0f;
constexpr float kGridStartY = 3.0f;
constexpr float kGridSpacing = 2.0f;

void Scene::Initialize()
{
    BA_LOG_INFO("Scene initialized.");
}

void Scene::Shutdown()
{
    m_gameObjects.clear();
    BA_LOG_INFO("Scene shutdown.");
}

uint32_t Scene::CreateGameObject()
{
    uint32_t id = m_nextId++;
    uint32_t index = static_cast<uint32_t>(m_gameObjects.size());

    GameObject gameObject;
    gameObject.id = id;

    const float* palette = kPalette[index % kPaletteSize];
    gameObject.color[0] = palette[0];
    gameObject.color[1] = palette[1];
    gameObject.color[2] = palette[2];
    gameObject.color[3] = palette[3];

    gameObject.transform.position.x = kGridStartX + static_cast<float>(index % kGridColumns) * kGridSpacing;
    gameObject.transform.position.y = kGridStartY - static_cast<float>(index / kGridColumns) * kGridSpacing;

    m_gameObjects.push_back(gameObject);

    BA_LOG_INFO("Created GameObject (ID: {})", id);
    return id;
}

void Scene::DestroyGameObject(uint32_t id)
{
    auto it = std::find_if(
        m_gameObjects.begin(),
        m_gameObjects.end(),
        [id](const GameObject& obj) { return (obj.id == id); }
    );

    BA_ASSERT(it != m_gameObjects.end());

    m_gameObjects.erase(it);
    BA_LOG_INFO("Destroyed GameObject (ID: {})", id);
}

std::span<const GameObject> Scene::GetGameObjects() const
{
    return m_gameObjects;
}

GameObject* Scene::FindGameObject(uint32_t id)
{
    auto it = std::find_if(
        m_gameObjects.begin(),
        m_gameObjects.end(),
        [id](const GameObject& obj) { return (obj.id == id); }
    );

    return (it != m_gameObjects.end()) ? &(*it) : nullptr;
}

std::unique_ptr<Scene> g_scene;

} // namespace BA
