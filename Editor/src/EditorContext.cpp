#include <EditorContext.hpp>
#include <SceneSystem/Node.hpp>

using namespace Droplet::Editor;

EditorContext::EditorContext(Droplet::Engine &p_engine)
	: m_engine(p_engine),
	  m_interactionState(),
	  m_editorCameraController(m_editorCamera)
{
	m_editorCamera.SetFOV(75.0f);
	
	m_engine.GetSceneManager().LoadScene("TestScene");
	SetActiveScene("TestScene");

	m_selectedScene = p_engine.GetSceneManager().GetScene("TestScene");
}

void EditorContext::SetActiveScene(std::string p_sceneName)
{
	auto newScene = m_engine.GetSceneManager().GetScene(p_sceneName);
	m_engine.GetSceneManager().ActivateScene(p_sceneName); // TODO: Set active is not set active??

	if (!newScene)
	{
		throw std::runtime_error("Could not load scene: " + p_sceneName);
	}
	
	m_selectedScene = std::move(newScene);
}

void EditorContext::CreateNewScene(std::string p_sceneName)
{
	m_engine.GetSceneManager().LoadScene(p_sceneName);
}

void EditorContext::SaveScene(const std::string &p_filepath)
{
	m_engine.GetSceneManager().SerializeToFile(GetActiveScene().lock(), p_filepath);
}

void EditorContext::LoadFromFile( const std::string &p_filepath)
{
	m_engine.GetSceneManager().LoadFromFile(p_filepath);
}

void EditorContext::SaveNode(const std::string &p_filepath, Droplet::Scene::Node &p_node)
{
	m_engine.GetSceneManager().SerializeNodeToFile(p_node, p_filepath);
}

void EditorContext::LoadNodeFromFile(std::string &p_filepath, std::optional<glm::vec3> p_overridePosition)
{
	GetActiveScene().lock()->LoadNodeFromFile(p_filepath, p_overridePosition);
}

Scene::EditorCamera &EditorContext::GetEditorCamera()
{
	return m_editorCamera;
}

Scene::EditorCameraController &EditorContext::GetEditorCameraController()
{
	return m_editorCameraController;
}

std::weak_ptr<Droplet::Scene::Scene> EditorContext::GetActiveScene()
{
	if (m_selectedScene.expired())
	{
		return {};
	}

	if (m_selectedScene.lock()->IsActive())
	{
		return m_selectedScene;
	}

	std::weak_ptr<Droplet::Scene::Scene> firstActiveScene = {};

	for (const auto &sceneWeak : m_engine.GetSceneManager().GetActiveScenes())
	{
		auto scene = sceneWeak.lock();
		if (!scene)
		{
			continue;
		}

		firstActiveScene = scene;
		break;
	}

	if (firstActiveScene.expired())
	{
		return m_selectedScene;
	}

	return firstActiveScene;
}

Droplet::Audio::AudioSystem& EditorContext::GetAudioSystem()
{
	return  m_engine.GetSceneManager().GetAudioSystem();
}

InteractionState &EditorContext::GetInteractionState()
{
	return m_interactionState;
}

Droplet::Engine &EditorContext::GetEngine()
{
	return m_engine;
}

void EditorContext::EnterPlayMode()
{
	if (m_isInPlayMode)
	{
		return; // Already in play mode
	}

	m_isInPlayMode = true;
	m_isPaused = false;

	// Fetch list of currently loaded and active scenes
	auto &sceneManager = m_engine.GetSceneManager();
	auto &loadedScenes = sceneManager.GetScenes();

	// Store the current state of each loaded scene
	m_sceneStatesOnPlay.clear();
	for (const auto &[name, scene] : loadedScenes)
	{
		SceneState &state = m_sceneStatesOnPlay[name];

		state.isActive = scene->IsActive();
		state.isMain = (scene == GetActiveScene().lock());

		state.json = state.serializer.SerializeScene(scene);
	}

	EngineFlagsOwner::SetUpdateFlags(UpdateFlags::PlayModeFlags);
}

void EditorContext::PausePlayMode()
{
	if (!m_isInPlayMode)
	{
		return; // Not in play mode
	}

	if (m_isPaused)
	{
		EngineFlagsOwner::SetUpdateFlags(UpdateFlags::PlayModeFlags);
		m_isPaused = false;
	}
	else
	{
		auto flags = UpdateFlags::EditorFlags;
		flags |= UpdateFlags::InPlayMode;		// Enable InPlayMode flag so that the game state is preserved when paused
		flags &= ~UpdateFlags::SkipAutoRender;	// Disable SkipAutoRender flag so that the game state is still rendered while paused

		EngineFlagsOwner::SetUpdateFlags(flags);
		m_isPaused = true;
	}
}

void EditorContext::ExitPlayMode()
{
	if (!m_isInPlayMode)
	{
		return; // Not in play mode
	}

	EngineFlagsOwner::SetUpdateFlags(UpdateFlags::EditorFlags);
	m_isInPlayMode = false;
	m_isPaused = false;

	auto &sceneManager = m_engine.GetSceneManager();

	// Unload all scenes
	std::unordered_map<std::string, std::shared_ptr<Droplet::Scene::Scene>> loadedScenes = sceneManager.GetScenes();
	if (!loadedScenes.empty())
	{
		for (const auto &[name, scene] : loadedScenes)
		{
			sceneManager.UnloadScene(name);
		}
	}

	// Restore to the state of each scene before entering play mode
	for (auto &[name, state] : m_sceneStatesOnPlay)
	{
		CreateNewScene(name);

		auto scene = sceneManager.GetScene(name);

		if (state.isActive != scene->IsActive())
		{
			if (state.isActive)
			{
				sceneManager.ActivateScene(name);
			}
			else
			{
				sceneManager.DeactivateScene(name);
			}
		}

		if (state.isMain)
		{
			SetActiveScene(name);
		}

		state.serializer.DeserializeScene(state.json, scene);
	}

	m_sceneStatesOnPlay.clear();
}
