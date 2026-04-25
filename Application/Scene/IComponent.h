#pragma once

namespace BA
{

class IComponent
{
public:
    virtual ~IComponent() = default;

    virtual bool IsEnabled() const = 0;
    virtual void SetEnabled(bool isEnabled) = 0;
};

} // namespace BA
