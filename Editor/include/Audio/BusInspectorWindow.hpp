#pragma once

#include "EditorWindow.hpp"
#include "EditorContext.hpp"
#include <Audio/AudioBus.hpp>

namespace Droplet::Editor
{

	/// @brief The Bus inspector window, interacting with bus specific operations
	/// operations such as lowering or increasing volume
	/// 
	/// TODO: Future implementations should also be able to attach, remove, and handle sound effects to the bus
	/// 
	class BusInspectorWindow : public EditorWindow
	{
	public:
		BusInspectorWindow(EditorContext *p_editorContext) : EditorWindow(p_editorContext) {};
		~BusInspectorWindow() = default;
		
		void SetCurrentBus(Audio::AudioBus *p_bus);
		void RenderEffectSlotDropTarget(Audio::AudioBus &p_bus, uint32_t p_slot);

	protected:

		void InitImpl() override;
		void CloseImpl() override;
		void RenderImpl() override;

	private:
		

		Audio::AudioBus *m_currentBus = nullptr;
	};
}