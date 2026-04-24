#pragma once

#include "Math/MathUtils.h"
#include "Scene/IComponent.h"
#include "Scene/LightComponent.h"
#include "Scene/ModelComponent.h"

namespace BA
{

struct GameObject
{
    uint32_t  id = 0;
    Transform transform;

    std::unordered_map<std::type_index, std::unique_ptr<IComponent>> components;

    GameObject() = default;
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;
    GameObject(GameObject&&) = default;
    GameObject& operator=(GameObject&&) = default;

    template <typename T>
    T* GetComponent()
    {
        auto it = components.find(std::type_index(typeid(T)));
        if (it == components.end())
        {
            return nullptr;
        }
        return static_cast<T*>(it->second.get());
    }

    template <typename T>
    const T* GetComponent() const
    {
        auto it = components.find(std::type_index(typeid(T)));
        if (it == components.end())
        {
            return nullptr;
        }
        return static_cast<const T*>(it->second.get());
    }

    template <typename T>
    bool HasComponent() const
    {
        return (components.find(std::type_index(typeid(T))) != components.end());
    }

    template <typename T, typename... Args>
    T& AddComponent(Args&&... args)
    {
        std::type_index key(typeid(T));
        BA_ASSERT(components.find(key) == components.end());
        auto owned = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *owned;
        components.emplace(key, std::move(owned));
        return ref;
    }

    template <typename T>
    void RemoveComponent()
    {
        components.erase(std::type_index(typeid(T)));
    }
};

} // namespace BA
