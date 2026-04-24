#pragma once

#include "Scene/IComponent.h"

namespace BA
{

struct ModelComponent : public IComponent
{
    std::string modelName;

    ModelComponent() = default;
    explicit ModelComponent(std::string name)
        : modelName(std::move(name))
    {
    }
};

} // namespace BA
