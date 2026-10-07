#include "Component.hpp"
#include <Node.hpp>
#include <ImGui/imgui.h>
#include <stdexcept>

using namespace Droplet::Scene;

void Component::SetOwner(std::shared_ptr<Node> p_owner)
{
    m_owner = p_owner;
}

Droplet::Scene::Component::Component()
{

}

void Component::RenderUI()
{
    using namespace ImGui;

    // TODO: Set active, delete

    RenderInternalUI();
}

std::shared_ptr<Node> Component::GetOwner() const
{
    return m_owner.lock();
}

bool Component::IsActive() const
{
	auto owner = m_owner.lock();
	if (!owner)
	{
		throw std::runtime_error("Component has no owner.");
	}

	return m_active && owner->IsActive();
}

void Component::SetActiveSelf(bool p_active)
{
	m_active = p_active;
}
