#include "TextComponent.hpp"

#include <Resource/ResourceManager.hpp>
#include <Engine/EngineFlags.hpp>
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>

Droplet::Scene::TextComponent::TextComponent()
{
	if (EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::EditorFlags))
	{
		ResourceManager &rsMan = ResourceManager::GetInstance();
		MetaEntry entry;
		
		if (rsMan.GetResourceMetaEntry("JetBrainsMono-Regular", entry))
		{
			SetFontGUID(entry.guid);
		}
	}
}

void Droplet::Scene::TextComponent::StartImpl()
{
	SetRenderable();
}

void Droplet::Scene::TextComponent::UpdateImpl()
{
	// TODO: MeshComponent has a todo here - I don't know
}

void Droplet::Scene::TextComponent::QueueRenderImpl()
{
	GetOwner()->GetScene()->QueueRenderable(shared_from_this());
}

