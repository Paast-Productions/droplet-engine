#pragma once

#include "EditorWindow.hpp"
#include "EditorContext.hpp"

namespace Droplet
{
	namespace Editor
	{
		/// @brief The Mixer Window, This window gives the same controls as the AudioMixer, 
		/// it controls the bus hierarchy, adding removing and routing buses, Also making buses dragable to other locations such as components
		/// 
		/// TODO: Future implementation would be to implement switching between different pre determined bus hierarchys, similiar to loading scenes
		/// 
		/// 
		class MixerWindow : public EditorWindow
		{
		public:
			explicit MixerWindow(EditorContext *p_editorContext) : EditorWindow(p_editorContext) {}

		protected:

			void InitImpl() override;
			void CloseImpl() override;
			void RenderImpl() override;
			void RenderToolbar() override;

		private:
			
			void DrawBus(Audio::AudioBus* p_bus);

			void HandleBusDropTarget(Audio::AudioBus *p_parent);

			std::vector<Audio::AudioBus *> m_visibleBuses;

			std::vector<std::pair<ImVec2, ImVec2>> m_visibleBusRects;
			bool m_marqueeActive = false;
			VkImageResolve2 m_marqueeStart{};

			std::vector<Audio::AudioBus *> m_selectionAnchor;

			std::weak_ptr<Engine> m_instance;

			std::vector<Audio::AudioBus *> m_nodesToReParent;

			struct PendingReparent
			{
				std::uint64_t busID;
				std::uint64_t newParentID;
			};


			std::optional<PendingReparent> m_pendingReparent;

			std::optional<std::uint64_t> m_pendingRemoveBusID;

			std::vector<Audio::AudioBus *> m_busesToRemove;
		};
	}

}

