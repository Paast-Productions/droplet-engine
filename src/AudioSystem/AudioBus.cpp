#include "Audio/AudioBus.hpp"
#include "Audio/AudioMixer.hpp"

using namespace Droplet::Audio;


Droplet::Audio::AudioBus::AudioBus(const std::string& p_name, uint64_t p_id) : m_name(p_name), m_id(p_id)
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
	// TODO Discuss (if gupta with arvid, if arvid with gupta) how bus hierachy affects each others volume
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
	return nlohmann::json();
}

void Droplet::Audio::AudioBus::DeserializeAudioBus(nlohmann::json p_json, std::weak_ptr<AudioBus> p_parentBus)
{
}
