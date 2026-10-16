#include <EditorContext.hpp>

Droplet::Editor::EditorContext::EditorContext(std::shared_ptr<Droplet::Engine> &p_engine)
	: m_sceneManager(p_engine->GetSceneManager()),
	  m_renderer(p_engine->GetRenderer()),
	  m_interactionState(),
	  m_editorCameraController(m_editorCamera)
{
	SetActiveScene("TestScene");
	m_selectedScene = p_engine->GetSceneManager().GetScene("TestScene");
}

void Droplet::Editor::EditorContext::SetActiveScene(std::string p_sceneName)
{
	if (m_selectedScene && m_selectedScene->GetName() == p_sceneName)
	{
		return;
	}
	
	m_sceneManager.LoadScene(p_sceneName);
	
	auto newScene = m_sceneManager.GetScene(p_sceneName);
	
	if (!newScene)
	{
		throw std::runtime_error("Could not load scene: " + p_sceneName);
	}
	
	m_selectedScene = std::move(newScene);
}

Droplet::Editor::Scene::EditorCamera &Droplet::Editor::EditorContext::GetEditorCamera()
{
	return m_editorCamera;
}

Droplet::Editor::Scene::EditorCameraController &Droplet::Editor::EditorContext::GetEditorCameraController()
{
	return m_editorCameraController;
}
std::shared_ptr<Droplet::Scene::Scene> Droplet::Editor::EditorContext::GetActiveScene()
{
	return m_selectedScene;
}
Droplet::Editor::InteractionState &Droplet::Editor::EditorContext::GetInteractionState()
{
	return m_interactionState;
}
Droplet::Graphics::Renderer &Droplet::Editor::EditorContext::GetRenderer()
{
	return m_renderer;
}