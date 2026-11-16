#include "AudioMixer.hpp"
#include "AudioBus.hpp"
using namespace Droplet::Audio;

Droplet::Audio::AudioMixer::AudioMixer()
{
}

void Droplet::Audio::AudioMixer::Initialize(dalia::Engine &p_dalia)
{
	m_daliaEngine = &p_dalia;
	const uint64_t masterID = m_nextBusID++;
	std::unique_ptr<AudioBus> master = std::make_unique<AudioBus>("Master", masterID);
	m_masterBus = master.get();
	m_buses.emplace(masterID, std::move(master));

	// Using 'Master' as our bus identifier for dalia gives this waring :
	// [DALIA WARNING] [API] CreateBus() - CreateBus called for existing bus (Master) with conflicting parent. Ignoring new parent.



	dalia::Result res;
	res = m_daliaEngine->CreateBus(m_masterBus->GetName().c_str()); // "Droplets master bus has the ID 0
	if (res != dalia::Result::Ok)
	{
		throw std::runtime_error("FAILED TO CREATE MASTER BUS");
	}
}

AudioBus* Droplet::Audio::AudioMixer::AddAudioBus(const std::string& p_name, AudioBus* p_parent)
{
	if (p_parent == nullptr)
	{
		throw std::invalid_argument("Audio bus parent cannot be nullptr");
	}

	if (HasAudioBusWithName(p_name))
	{
		return nullptr;
	}

	const auto parentIT = m_buses.find(p_parent->GetID());
	if (parentIT == m_buses.end())
	{
		throw std::invalid_argument("None existing parent");
	}

	const uint64_t busID = m_nextBusID;

	std::unique_ptr<AudioBus> newBus = std::make_unique<AudioBus>(p_name, busID);

	AudioBus *busPointer = newBus.get();

	dalia::Result res;
	res = m_daliaEngine->CreateBus(busPointer->GetName().c_str());
	if (res == dalia::Result::BusCapacityReached)
	{
		// Maximum number of buses are 63
		throw std::runtime_error("Bus pool exhausted");
	}

	const auto [iterator, inserted] = m_buses.emplace(busID, std::move(newBus));

	if (!inserted)
	{
		throw std::runtime_error("failed to add audiobus due to duplicate ID");
	}

	SetBusParent(busPointer, p_parent);

	m_nextBusID++;

	return busPointer;
}

void Droplet::Audio::AudioMixer::RemoveAudioBus(uint64_t p_id)
{
	const auto busIt = m_buses.find(p_id);

	if (busIt == m_buses.end())
	{
		throw std::runtime_error("Tried to remove nonexisting bus");
	}

	AudioBus *bus = busIt->second.get();

	if (bus == m_masterBus)
	{
		throw std::invalid_argument("Cannot remove the master bus");
	}
	AudioBus *newParent = bus->m_parent;

	if (newParent == nullptr)
	{
		newParent = m_masterBus;
	}

	const std::vector<AudioBus *> children = bus->m_children;

	for (AudioBus *child : children)
	{
		SetBusParent(child, newParent);
	}

	const auto childIt = std::find(newParent->m_children.begin(), newParent->m_children.end(), bus);

	if (childIt != newParent->m_children.end())
	{
		newParent->m_children.erase(childIt);
	}

	bus->m_parent = nullptr;


	m_daliaEngine->DestroyBus(bus->GetName().c_str());
	m_buses.erase(busIt);

}

void Droplet::Audio::AudioMixer::SetBusParent(AudioBus *p_childBus, AudioBus *p_parentBus)
{
	if (p_childBus == nullptr || p_parentBus == nullptr)
	{
		throw std::invalid_argument("Bus cannot be nullptr");
	}

	const auto busIt = m_buses.find(p_childBus->GetID());
	const auto parentIt = m_buses.find(p_parentBus->GetID());

	if (busIt == m_buses.end() || busIt->second.get() != p_childBus)
	{
		throw std::runtime_error("Child bus couldn't be found");
	}

	if (parentIt == m_buses.end() || parentIt->second.get() != p_parentBus)
	{
		throw std::runtime_error("Parent bus couldn't be found");
	}

	if (p_childBus == m_masterBus)
	{
		throw std::invalid_argument("Cannot parent / reparent the masterbus");
	}

	if (p_childBus->m_parent == p_parentBus)
	{
		return; // Early return
	}

	p_parentBus->m_children.push_back(p_childBus);

	if (p_childBus->m_parent != nullptr)
	{
		AudioBus *oldParent = p_childBus->m_parent;

		const auto childIt = std::find(oldParent->m_children.begin(), oldParent->m_children.end(), p_childBus);
		if (childIt == oldParent->m_children.end())
		{
			p_parentBus->m_children.pop_back();
			throw std::runtime_error("Bus hierarchy is inconsistent");
		}

		oldParent->m_children.erase(childIt);
	}
	p_childBus->m_parent = p_parentBus;


	m_daliaEngine->RouteBus(p_childBus->GetName().c_str(), p_parentBus->GetName().c_str());
}

bool Droplet::Audio::AudioMixer::HasAudioBusWithName(const std::string &p_name)
{
	for (const auto &[id, bus] : m_buses)
	{
		if (bus->GetName() == p_name)
		{
			return true;
		}
	}
	return false;
}

AudioBus *Droplet::Audio::AudioMixer::FindAudioBus(std::uint64_t p_id) const
{
	const auto it = m_buses.find(p_id);
	if (it == m_buses.end())
	{
		return nullptr;
	}
	return it->second.get();
}
