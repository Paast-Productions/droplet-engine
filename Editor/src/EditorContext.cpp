#include <EditorContext.hpp>
#include <SceneSystem/Node.hpp>

Droplet::Editor::EditorContext::EditorContext(Droplet::Engine &p_engine)
	: m_engine(p_engine),
	  m_interactionState(),
	  m_editorCameraController(m_editorCamera)
{
	m_editorCamera.SetFOV(75.0f);
	
	m_engine.GetSceneManager().LoadScene("TestScene");
	SetActiveScene("TestScene");

	m_selectedScene = p_engine.GetSceneManager().GetScene("TestScene");
}

void Droplet::Editor::EditorContext::SetActiveScene(std::string p_sceneName)
{
	auto newScene = m_engine.GetSceneManager().GetScene(p_sceneName);
	m_engine.GetSceneManager().ActivateScene(p_sceneName); // TODO: Set active is not set active??

	if (!newScene)
	{
		throw std::runtime_error("Could not load scene: " + p_sceneName);
	}
	
	m_selectedScene = std::move(newScene);
}

void Droplet::Editor::EditorContext::CreateNewScene(std::string p_sceneName)
{
	m_engine.GetSceneManager().LoadScene(p_sceneName);
}

void Droplet::Editor::EditorContext::SaveScene(const std::string &p_filepath)
{
	m_engine.GetSceneManager().SerializeToFile(GetActiveScene().lock(), p_filepath);
}

void Droplet::Editor::EditorContext::LoadFromFile( const std::string &p_filepath)
{
	m_engine.GetSceneManager().LoadFromFile(p_filepath);
}

void Droplet::Editor::EditorContext::SaveNode(const std::string &p_filepath, const Droplet::Scene::Node &p_node)
{
	m_engine.GetSceneManager().SerializeNodeToFile(p_node, p_filepath);
}

void Droplet::Editor::EditorContext::LoadNodeFromFile(std::string &p_filepath, std::optional<glm::vec3> p_overridePosition)
{
	GetActiveScene().lock()->LoadNodeFromFile(p_filepath,p_overridePosition);
}

Droplet::Editor::Scene::EditorCamera &Droplet::Editor::EditorContext::GetEditorCamera()
{
	return m_editorCamera;
}

Droplet::Editor::Scene::EditorCameraController &Droplet::Editor::EditorContext::GetEditorCameraController()
{
	return m_editorCameraController;
}

std::weak_ptr<Droplet::Scene::Scene> Droplet::Editor::EditorContext::GetActiveScene()
{
	return m_selectedScene;
}

Droplet::Audio::AudioSystem& Droplet::Editor::EditorContext::GetAudioSystem()
{
	return  m_engine.GetSceneManager().GetAudioSystem();
}

Droplet::Editor::InteractionState &Droplet::Editor::EditorContext::GetInteractionState()
{
	return m_interactionState;
}

Droplet::Engine &Droplet::Editor::EditorContext::GetEngine()
{
	return m_engine;
}
