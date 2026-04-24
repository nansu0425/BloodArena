#pragma once

#include "Math/MathUtils.h"
#include "Scene/IComponent.h"
#include "Scene/LightComponent.h"
#include "Scene/ModelComponent.h"

namespace BA
{

class GameObject
{
public:
    GameObject() = default;
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;
    GameObject(GameObject&&) = default;
    GameObject& operator=(GameObject&&) = default;

    uint32_t         GetId() const;
    void             SetId(uint32_t id);

    const Transform& GetTransform() const;
    Transform&       GetTransform();
    void             SetTransform(const Transform& transform);

    template <typename T>
    T* GetComponent()
    {
        auto it = m_components.find(std::type_index(typeid(T)));
        if (it == m_components.end())
        {
            return nullptr;
        }
        return static_cast<T*>(it->second.get());
    }

    template <typename T>
    const T* GetComponent() const
    {
        auto it = m_components.find(std::type_index(typeid(T)));
        if (it == m_components.end())
        {
            return nullptr;
        }
        return static_cast<const T*>(it->second.get());
    }

    template <typename T>
    bool HasComponent() const
    {
        return (m_components.find(std::type_index(typeid(T))) != m_components.end());
    }

    template <typename T, typename... Args>
    T& AddComponent(Args&&... args)
    {
        std::type_index key(typeid(T));
        BA_ASSERT(m_components.find(key) == m_components.end());
        auto owned = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *owned;
        m_components.emplace(key, std::move(owned));
        return ref;
    }

    template <typename T>
    void RemoveComponent()
    {
        m_components.erase(std::type_index(typeid(T)));
    }

private:
    uint32_t  m_id = 0;
    Transform m_transform;

    std::unordered_map<std::type_index, std::unique_ptr<IComponent>> m_components;
};

} // namespace BA
