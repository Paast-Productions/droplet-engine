#include "Component.hpp"

using namespace Droplet::Scene;

void Component::SetOwner(std::shared_ptr<Node> p_owner)
{
    m_owner = p_owner;
}

std::shared_ptr<Node> Component::GetOwner() const
{
    return m_owner.lock();
}
