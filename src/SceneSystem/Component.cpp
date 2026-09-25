#include "Component.hpp"
#include <Node.hpp>

using namespace Droplet::Scene;

void Component::SetOwner(std::shared_ptr<Node> p_owner)
{
    m_owner = p_owner;
}

void Component::RenderUI()
{
    // TODO: implement component wrapper UI

    RenderInternalUI();

    // TODO: implement component wrapper UI
}

std::shared_ptr<Node> Component::GetOwner() const
{
    return m_owner.lock();
}
