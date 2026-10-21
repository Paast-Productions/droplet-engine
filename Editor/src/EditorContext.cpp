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
	if (m_selectedScene && m_selectedScene->GetName() == p_sceneName)
	{
		return;
	}
	
	//m_engine.GetSceneManager().LoadScene(p_sceneName);
	
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
	m_engine.GetSceneManager().SerializeToFile(GetActiveScene(), p_filepath);
}

void Droplet::Editor::EditorContext::LoadFromFile( std::string &p_filepath)
{
	m_engine.GetSceneManager().LoadFromFile(p_filepath);
}

void Droplet::Editor::EditorContext::SaveNode(const std::string &p_filepath, const Droplet::Scene::Node &p_node)
{
	m_engine.GetSceneManager().SerializeNodeToFile(p_node, p_filepath);
}

void Droplet::Editor::EditorContext::LoadNodeFromFile(std::string &p_filepath)
{
	GetActiveScene()->LoadNodeFromFile(p_filepath);
}

Droplet::Editor::Scene::EditorCamera &Droplet::Editor::EditorContext::GetEditorCamera()
{
	return m_editorCamera;
}

Droplet::Editor::Scene::EditorCameraController &Droplet::Editor::EditorContext::GetEditorCameraController()
{
	return m_editorCameraController;
}
std::shared_ptr<Droplet::Scene::Scene> Droplet::Editor::EditorContext::GetActiveScene() // TODO: Make weak pointer
{
	return m_selectedScene;
}
Droplet::Editor::InteractionState &Droplet::Editor::EditorContext::GetInteractionState()
{
	return m_interactionState;
}
Droplet::Graphics::Renderer &Droplet::Editor::EditorContext::GetRenderer()
{
	return m_engine.GetRenderer();
}
Droplet::ResourceManager &Droplet::Editor::EditorContext::GetResourceManager()
{
	return m_engine.GetResourceManager();
}
Droplet::Engine &Droplet::Editor::EditorContext::GetEngine()
{
	return m_engine;
}