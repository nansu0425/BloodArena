#pragma once

namespace BA
{

class GameObject;

class ITickable
{
public:
    virtual ~ITickable() = default;

    virtual void Tick(float deltaSeconds, GameObject& owner) = 0;
};

} // namespace BA
