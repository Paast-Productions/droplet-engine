#pragma once
#include <dalia/dalia.h>
#include "AudioMixer.hpp"

namespace Droplet::Audio
{
	class AudioSystem
	{
	public:
		AudioSystem();
		~AudioSystem();

		void ShutDown();
		AudioMixer &GetMixer();

		void PlaySound();
		void PauseSound();

		void Initialize();
		


	private:

		dalia::Engine m_daliaEngine;
		AudioMixer m_mixer;
	};
}
