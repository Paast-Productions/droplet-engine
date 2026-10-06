#include <EditorContext.hpp>

Droplet::Editor::EditorContext::EditorContext()
	: m_interactionState(),
	  m_editorCamera(m_interactionState){}

void Droplet::Editor::EditorContext::SetScene(std::shared_ptr<Droplet::Scene::Scene> p_scene)
{
	m_scene = p_scene;
}

Droplet::Editor::Scene::EditorCamera &Droplet::Editor::EditorContext::GetEditorCamera()
{
	return m_editorCamera;
}
std::shared_ptr<Droplet::Scene::Scene> Droplet::Editor::EditorContext::GetScene()
{
	return m_scene;
}
Droplet::Editor::InteractionState &Droplet::Editor::EditorContext::GetInteractionState()
{
	return m_interactionState;
}