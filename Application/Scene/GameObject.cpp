#include "Core/PCH.h"
#include "Scene/GameObject.h"

namespace BA
{

uint32_t GameObject::GetId() const
{
    return m_id;
}

void GameObject::SetId(uint32_t id)
{
    m_id = id;
}

const std::string& GameObject::GetName() const
{
    return m_name;
}

void GameObject::SetName(std::string name)
{
    m_name = std::move(name);
}

const Transform& GameObject::GetTransform() const
{
    return m_transform;
}

Transform& GameObject::GetTransform()
{
    return m_transform;
}

void GameObject::SetTransform(const Transform& transform)
{
    m_transform = transform;
}

} // namespace BA
