#pragma once

#include "Scene/IComponent.h"

namespace BA
{

class ModelComponent : public IComponent
{
public:
    ModelComponent() = default;
    explicit ModelComponent(std::string modelName);

    bool IsEnabled() const override;
    void SetEnabled(bool isEnabled) override;

    const std::string& GetModelName() const;
    void SetModelName(std::string modelName);

private:
    std::string m_modelName;
    bool        m_isEnabled = true;
};

} // namespace BA
