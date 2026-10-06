#include "Scene/EditorCamera.hpp"
#include "InteractionState.hpp"
#include "../../include/SceneSystem/Scene.hpp"

class EditorContext {

private:

	EditorCamera m_editorCamera;
	std::shared_ptr<InteractionState> m_interactionState;
	std::shared_ptr<Scene> m_scene;
};