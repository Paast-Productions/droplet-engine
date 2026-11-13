#include "Audio/AudioBus.hpp"
#include "Audio/AudioMixer.hpp"
#include "Audio/EffectFactory.hpp"
#include "Audio/Effect.hpp"
#include "Audio/AudioSystem.hpp"


using namespace Droplet::Audio;


Droplet::Audio::AudioBus::AudioBus(const std::string& p_name, uint64_t p_id) : m_name(p_name), m_id(p_id)
{
}

Droplet::Audio::AudioBus::~AudioBus()
{
}

void Droplet::Audio::AudioBus::SetActive(bool p_active)
{
	m_active = p_active;
}

bool Droplet::Audio::AudioBus::IsActive() const
{
	if (!m_active)
	{
		return false;
	}

	if (m_parent != nullptr)
	{
		return m_parent->IsActive();
	}

	return true;
}

void Droplet::Audio::AudioBus::SetVolume(float p_volume)
{
	m_volume = p_volume;

}

void Droplet::Audio::AudioBus::SetName(std::string p_name)
{
	m_name = p_name;
}

void Droplet::Audio::AudioBus::SetParent(AudioBus* p_audioBus)
{
	m_parent = p_audioBus;
}

AudioBus* Droplet::Audio::AudioBus::GetParent() const
{
	return m_parent;
}

AudioBus* Droplet::Audio::AudioBus::AddChildBus(AudioBus* p_audioBus)
{
	if (!p_audioBus)
	{
		throw std::invalid_argument("Cannot add nullptr as a child audiobus");
	}

	if (p_audioBus == this)
	{
		throw std::invalid_argument("cannot ass AudioBus '" + m_name + "' as a child of itself");
	}
	AudioBus *parent = m_parent;
	if (parent != nullptr)
	{
		while (parent)
		{
			if (parent == p_audioBus)
			{
				throw std::runtime_error("Cannot add Node '" + p_audioBus->GetName() + "' as a child of '" + m_name + "': Adding a audiobus as a child of its own descendent would create a cycle");
			}
			parent = parent->GetParent();
		}
	}

	if (AudioBus* prevParent = p_audioBus->GetParent())
	{
		prevParent->RemoveChildBus(p_audioBus);
	}
	else
	{
		//TODO Handle master bus here
	}

	p_audioBus->m_parent = this;

	m_children.push_back(p_audioBus);

	return p_audioBus;
}

void Droplet::Audio::AudioBus::RemoveChildBus(const AudioBus* p_audioBus)
{
	if (!p_audioBus)
	{
		throw std::invalid_argument("Cannot remove nullptr as a child audio bus");
	}

	if (p_audioBus == this)
	{
		throw std::runtime_error("Cannot remove audio bus, Audiobus is not a child of:'" + m_name + "'.");
	}

	auto iterator = std::find(m_children.begin(), m_children.end(), p_audioBus);

	if (iterator == m_children.end())
	{
		throw std::runtime_error("Cannot remove audiobus '" + p_audioBus->GetName() + "': Audio bus is not child of '" + m_name);
	}

	m_children.erase(iterator);

	// TODO Either add the child to the master bus, or just erase it totally
}

bool Droplet::Audio::AudioBus::IsDecendantOf(const AudioBus* p_audioBus) const
{
	AudioBus *parent = m_parent;

	while (parent != nullptr)
	{
		if (parent == p_audioBus)
		{
			return true;
		}
		parent = parent->GetParent();
	}
	return false;
}

const std::vector<AudioBus *> & AudioBus::GetChildren() const
{
	return m_children;
}

void Droplet::Audio::AudioBus::RenderUI()
{
	ZoneScoped;
	ZoneText(m_name.c_str(), m_name.size());
}

nlohmann::json Droplet::Audio::AudioBus::SerializeAudioBus()
{
	nlohmann::json busJson;

	busJson["name"] = m_name;
	busJson["id"] = m_id;
	busJson["active"] = m_active;
	busJson["volume"] = m_volume;

	busJson["effects"] = nlohmann::json::array();

	for (int i = 0; i < MAX_EFFECTS; ++i)
	{
		if (m_effects[i])
		{
			busJson["effects"].push_back(m_effects[i]->Serialize());
			
		}
	}

	busJson["children"] = nlohmann::json::array();

	for (auto &child : m_children)
	{
		busJson["children"].push_back(child->SerializeAudioBus());
	}

	return busJson;
}

void Droplet::Audio::AudioBus::DeserializeAudioBus(nlohmann::json p_json, AudioBus *p_parent)
{
	
}

Effect *Droplet::Audio::AudioBus::AttachEffect(const std::string &p_effectType)
{
	std::unique_ptr<Effect> effect = EffectFactory::CreateEffect(p_effectType);
	return AttachEffect(std::move(effect));
}

Effect *Droplet::Audio::AudioBus::AttachEffect(std::unique_ptr<Effect> p_effect, std::optional<uint32_t> p_effecSlot)
{
	ZoneScoped;

	if (!p_effect)
	{
		throw std::runtime_error("Cannot add a null effect");
	}

	uint32_t slot = MAX_EFFECTS + 1;

	bool foundSlot = false;

	if (!p_effecSlot.has_value())
	{
		for (uint32_t i = 0; i < MAX_EFFECTS; ++i)
		{
			if (!m_effects[i])
			{
				slot = i;
				break;
			}
		}
	}
	else
	{
		slot = p_effecSlot.value();

	}
	
	if (slot >= MAX_EFFECTS)
	{
		return nullptr;
	}
	p_effect->SetSlot(slot);
	m_effects[slot] = std::move(p_effect); // HACK This would replace the effect in the spot that is selected, do we want this or should we reject new effect?
	
	if (m_started)
	{
		m_effects[slot].get()->Start();
	}

	m_effects[slot]->SetOwner(this);
	if (!AudioSystem::GetInstance().AttachEffect(m_effects[slot]->GetEffectHandle(), m_name, slot))
	{
		return nullptr;
	}


	return m_effects[slot].get();
}

void Droplet::Audio::AudioBus::RemoveEffect(Effect *p_effect)
{
	if (!p_effect)
	{
		return;
	}
	uint32_t slot = MAX_EFFECTS + 1;
	for (uint32_t i = 0; i < MAX_EFFECTS; ++i)
	{
		if (m_effects[i].get() == p_effect)
		{
			slot = i;
			break;
		}
	}

	if (slot >= MAX_EFFECTS)
	{
		return;
	}

	if (!Audio::AudioSystem::GetInstance().DetachEffect(p_effect->GetEffectHandle()))
	{
		return;
	}

	if (!Audio::AudioSystem::GetInstance().DestroyEffect(p_effect->GetEffectHandle()))
	{
		return;
	}

	m_effects[slot].reset();

}

void Droplet::Audio::AudioBus::RemoveEffect(uint32_t p_slot)
{
	if (p_slot >= MAX_EFFECTS)
	{
		return;
	}

	if (!m_effects[p_slot])
	{
		return;
	}

	if (!AudioSystem::GetInstance().DetachEffect(m_effects[p_slot]->GetEffectHandle()))
	{
		return;
	}

	m_effects[p_slot].reset();
}

void Droplet::Audio::AudioBus::MoveEffect(uint32_t p_fromSlot, uint32_t p_toSlot)
{
	if (!m_effects[p_fromSlot])
	{
		//can't move nullptr;
		return;
	}

	if (p_toSlot >= MAX_EFFECTS)
	{
		return;
	}

	if (!m_effects[p_toSlot]) // The slot was empty
	{
		if (!AudioSystem::GetInstance().DetachEffect(m_effects[p_fromSlot]->GetEffectHandle()))
		{
			std::cout << "Could not detach effect" << std::endl;
			return;
		}
		m_effects[p_toSlot] = std::move(m_effects[p_fromSlot]);
		if (!AudioSystem::GetInstance().AttachEffect(m_effects[p_toSlot]->GetEffectHandle(),m_name,p_toSlot))
		{
			std::cout << "Could not attach effect after detaching" << std::endl;
			return;
		}
		m_effects[p_toSlot]->SetSlot(p_toSlot);
	}
	else // Slot was not empty, switch places on the effects
	{
		Effect *fromEffect = m_effects[p_fromSlot].get();
		Effect *toEffect = m_effects[p_toSlot].get();

		if (!AudioSystem::GetInstance().DetachEffect(fromEffect->GetEffectHandle()))
		{
			std::cout << "Could not detach effect" << std::endl;
			return;
		}
		if (!AudioSystem::GetInstance().DetachEffect(toEffect->GetEffectHandle()))
		{
			std::cout << "Could not detach effect" << std::endl;
			return;
		}

		if (!AudioSystem::GetInstance().AttachEffect(fromEffect->GetEffectHandle(), m_name, p_toSlot))
		{
			std::cout << "Could not attach effect after detaching" << std::endl;
			return;
		}

		if (!AudioSystem::GetInstance().AttachEffect(toEffect->GetEffectHandle(), m_name, p_fromSlot))
		{
			std::cout << "Could not attach effect after detaching" << std::endl;
			return;
		}

		m_effects[p_fromSlot].swap(m_effects[p_toSlot]);

		m_effects[p_fromSlot]->SetSlot(p_fromSlot);
		m_effects[p_toSlot]->SetSlot(p_toSlot);
	}
	
}

Effect *Droplet::Audio::AudioBus::GetEffect(uint32_t p_slot)
{
	return m_effects[p_slot].get();
}
