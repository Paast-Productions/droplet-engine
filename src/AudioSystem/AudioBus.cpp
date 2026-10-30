#include "AudioBus.hpp"

using namespace Droplet::Audio;


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

	if (auto parent = m_parent.lock())
	{
		return parent->IsActive();
	}
}

void Droplet::Audio::AudioBus::SetName(std::string p_name)
{
	m_name = p_name;
}

void Droplet::Audio::AudioBus::SetParent(std::weak_ptr<AudioBus> p_audioBus)
{
	m_parent = p_audioBus;
}

std::weak_ptr<AudioBus> Droplet::Audio::AudioBus::GetParent() const
{
	return m_parent;
}

std::weak_ptr<AudioBus> Droplet::Audio::AudioBus::AddChildBus(std::shared_ptr<AudioBus> p_audioBus)
{
	if (!p_audioBus)
	{
		throw std::invalid_argument("Cannot add nullptr as a child audiobus");
	}

	if (p_audioBus.get() == this)
	{
		throw std::invalid_argument("cannot ass AudioBus '" + m_name + "' as a child of itself");
	}

	if (auto parent = GetParent().lock())
	{
		while (parent)
		{
			if (parent == p_audioBus)
			{
				throw std::runtime_error("Cannot add Node '" + p_audioBus->GetName() + "' as a child of '" + m_name + "': Adding a audiobus as a child of its own descendent would create a cycle");
			}
			parent = parent->GetParent().lock();
		}
	}

	if (auto prevParent = p_audioBus->GetParent().lock())
	{

	}



	return std::weak_ptr<AudioBus>();
}

void Droplet::Audio::AudioBus::RemoveChildBus(const std::weak_ptr<AudioBus> p_audioBus)
{
}

bool Droplet::Audio::AudioBus::IsDecendantOf(const std::weak_ptr<AudioBus> &p_audioBus) const
{
	return false;
}

void Droplet::Audio::AudioBus::RenderUI()
{

}

nlohmann::json Droplet::Audio::AudioBus::SerializeAudioBus()
{
	return nlohmann::json();
}

void Droplet::Audio::AudioBus::DeserializeAudioBus(nlohmann::json p_json, std::weak_ptr<AudioBus> p_parentBus)
{
}
