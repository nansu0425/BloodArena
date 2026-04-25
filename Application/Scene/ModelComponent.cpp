#include "Core/PCH.h"
#include "Scene/ModelComponent.h"

namespace BA
{

ModelComponent::ModelComponent(std::string modelName)
    : m_modelName(std::move(modelName))
{
}

bool ModelComponent::IsEnabled() const
{
    return m_isEnabled;
}

void ModelComponent::SetEnabled(bool isEnabled)
{
    m_isEnabled = isEnabled;
}

const std::string& ModelComponent::GetModelName() const
{
    return m_modelName;
}

void ModelComponent::SetModelName(std::string modelName)
{
    m_modelName = std::move(modelName);
}

} // namespace BA
