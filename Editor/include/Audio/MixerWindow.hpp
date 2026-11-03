#pragma once

#include "EditorWindow.hpp"
#include "EditorContext.hpp"

namespace Droplet
{

	namespace Editor
	{


		class MixerWindow : public EditorWindow
		{
		public:
			explicit MixerWindow(EditorContext *p_editorContext);

		protected:

			void InitImpl() override;
			void CloseImpl() override;
			void RenderImpl() override;
			void RenderToolbar() override;

		private:
			
			void DrawBus(Audio::AudioBus* p_bus);

			void HandleBusDropTarget(Audio::AudioBus *p_parent);

			EditorContext *m_editorContext;

			std::vector<Audio::AudioBus *> m_visibleBuses;

			std::vector<std::pair<ImVec2, ImVec2>> m_visibleBusRects;
			bool m_marqueeActive = false;
			VkImageResolve2 m_marqueeStart{};

			std::vector<Audio::AudioBus *> m_selectionAnchor;

			Audio::AudioBus *m_rangeselectTarget;

			Audio::AudioBus *m_pendingCollapseBus;

			bool m_rangeSelectAdditive = false;

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

