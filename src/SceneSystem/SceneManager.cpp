#include "SceneManager.hpp"
#include <SceneFactory.hpp>
#include <SceneSerializer.hpp>
#include <Scene.hpp>
#include <Behaviour.hpp>

#include <Core/IoManager.hpp>
#include <Core/ThreadPool.hpp>
#include <tracy/public/tracy/Tracy.hpp>
#include <iostream>
#include <EngineFlags.hpp>
#include <Time.hpp>
#include <Common/Camera.hpp>
#include <SceneSystem/Components/CameraComponent.hpp>

using namespace Droplet::Scene;

std::weak_ptr<Scene> SceneManager::LoadScene(const std::string &p_name)
{
    ZoneScoped;
    ZoneText(p_name.c_str(), p_name.size());

    if (m_scenes.contains(p_name))
    {
        UnloadScene(p_name);
        //throw std::runtime_error("Cannot load Scene '" + p_name + "': Scene already exists.");
    }

    std::shared_ptr<Scene> scene = SceneFactory::CreateScene(p_name);
	scene->SetSceneManager(this);

    if (!scene)
    {
        throw std::runtime_error("Cannot load Scene '" + p_name + "': SceneFactory failed to create the Scene.");
    }

    scene->Load();
    
    m_scenes[p_name] = scene;
    return scene;
}

void SceneManager::UnloadScene(const std::string &p_name)
{
    ZoneScoped;
    ZoneText(p_name.c_str(), p_name.size());

    auto it = m_scenes.find(p_name);

    if (it == m_scenes.end())
    {
        throw std::runtime_error("Cannot unload Scene '" + p_name + "': Scene does not exist.");
    }

    if (it->second->IsActive())
    {
        DeactivateScene(p_name);
    }

    it->second->Unload();

    m_scenes.erase(it);
}

void SceneManager::UnloadSceneAsync(const std::string &p_name, std::function<void()> p_onUnloaded)
{
    ZoneScoped;
    ZoneText(p_name.c_str(), p_name.size());

    auto it = m_scenes.find(p_name);

    if (it == m_scenes.end())
    {
        throw std::runtime_error("Cannot unload Scene '" + p_name + "': Scene does not exist.");
    }

    std::shared_ptr<Scene> scene = it->second;

    if (scene->IsActive())
    {
        DeactivateScene(p_name);
    }

    scene->Unload();
    m_scenes.erase(it);

    ++m_pendingAsyncSceneUnloads;

    // Dropping the last reference here destroys the Scene on a worker thread
    ThreadPool::GetInstance().PushTask([this, scene = std::move(scene), onUnloaded = std::move(p_onUnloaded)]() mutable
    {
        scene.reset();
        m_asyncSceneUnloadResults.Push(AsyncSceneUnloadResult{ std::move(onUnloaded) });
    });
}

bool SceneManager::HasPendingAsyncUnloads() const
{
    return m_pendingAsyncSceneUnloads.load() > 0;
}

void SceneManager::FinalizeAsyncSceneUnloads()
{
    ZoneScoped;

    AsyncSceneUnloadResult result;
    while (m_asyncSceneUnloadResults.Pop(result))
    {
        --m_pendingAsyncSceneUnloads;

        if (result.onUnloaded)
        {
            result.onUnloaded();
        }
    }
}

void SceneManager::ActivateScene(const std::string &p_name)
{
    ZoneScoped;
    ZoneText(p_name.c_str(), p_name.size());

    auto scene = GetScene(p_name);

    if (!scene)
    {
        throw std::runtime_error("Cannot activate Scene '" + p_name + "': Scene does not exist.");
    }

    if (!scene->IsLoaded())
    {
        throw std::runtime_error("Cannot activate Scene '" + p_name + "': Scene is not loaded.");
    }

    if (scene->IsActive())
    {
		return; // Scene is already active
    }

    scene->SetActive(true);

    m_activeScenes.push_back(scene);
}

void SceneManager::DeactivateScene(const std::string &p_name)
{
    ZoneScoped;
    ZoneText(p_name.c_str(), p_name.size());

    auto scene = GetScene(p_name);

    if (!scene)
    {
        throw std::runtime_error("Cannot deactivate Scene '" + p_name + "': Scene does not exist.");
    }

    if (!scene->IsLoaded())
    {
        throw std::runtime_error("Cannot deactivate Scene '" + p_name + "': Scene is not loaded.");
    }

    if (!scene->IsActive())
    {
		return; // Scene is already inactive
    }

    scene->SetActive(false);

    m_activeScenes.erase(std::remove_if(m_activeScenes.begin(), m_activeScenes.end(), [&p_name](const std::weak_ptr<Scene> &p_scene)
    {
        auto scene = p_scene.lock();

        if (!scene)
        {
            return true;
        }

        if (scene->GetName() == p_name)
        {
            return true;
        }

        return false;
    }
    ),
    m_activeScenes.end());
}

std::shared_ptr<Scene> SceneManager::GetScene(const std::string &p_name) const // TODO: (SCENE) Make weak pointer
{
    auto it = m_scenes.find(p_name);

    if (it == m_scenes.end())
    {
        return nullptr;
    }

    return it->second;
}

const std::vector<std::weak_ptr<Scene>> &SceneManager::GetActiveScenes() const
{
    return m_activeScenes;
}

void SceneManager::Update()
{
    ZoneScoped;

    FinalizeAsyncSceneLoads();
    FinalizeAsyncSceneUnloads();

    Droplet::Time &time = Droplet::Time::Get();

	// Pre-Update all active Behaviours
    if (!EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::SkipBehaviourLogic))
    {
		ZoneNamedNC(behaviourPreUpdateZone, "Behaviour Pre-Update", tracy::Color::Green, true);

        // Iterate a snapshot so behaviours can be started or shut down during the update
        std::vector<std::shared_ptr<Behaviour>> behaviours = GetBehaviours();
        for (auto it = behaviours.begin(); it != behaviours.end(); )
        {
            auto &behaviour = *it;

            if (behaviour->IsActive())
            {
                ZoneNamedN(preUpdateZone, "Pre-Update", true);
                ZoneTextV(preUpdateZone, behaviour->GetTypeName().data(), behaviour->GetTypeName().size());

                behaviour->PreUpdate();
            }

            ++it;
        }
    }

    // Copy the active scenes to avoid iterator invalidation
    std::vector<std::weak_ptr<Scene>> activeScenesCopy = m_activeScenes; 

	// Update all active Scenes
    {
        ZoneNamedNC(sceneUpdateZone, "Scene Update", tracy::Color::DarkSeaGreen1, true);

        for (int i = 0; i < activeScenesCopy.size(); ++i)
        {
            auto scene = activeScenesCopy[i].lock();
            if (!scene)
            {
                continue;
            }

            scene->Update();
        }
    }

	// Fixed-Update all active Scenes
    while (time.GetFixedTimeAccumulator() >= time.GetFixedDeltaTime())
    {
        ZoneNamedNC(sceneFixedUpdateZone, "Scene Fixed Update", tracy::Color::Orange, true);

        for (int i = 0; i < activeScenesCopy.size(); ++i)
        {
            auto scene = activeScenesCopy[i].lock();
            if (!scene)
            {
                continue;
            }

            scene->FixedUpdate();
        }

        time.ConsumeFixedTime();
    }

	// Late-Update all active Scenes
    {
		ZoneNamedNC(sceneLateUpdateZone, "Scene Late Update", tracy::Color::Blue, true);

        for (int i = 0; i < activeScenesCopy.size(); ++i)
        {
            auto scene = activeScenesCopy[i].lock();
            if (!scene)
            {
                continue;
            }

            scene->LateUpdate();
        }
    }

    // Post-Update all active Behaviours
    if (!EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::SkipBehaviourLogic))
    {
		ZoneNamedNC(behaviourPostUpdateZone, "Behaviour Post-Update", tracy::Color::Red, true);

        std::vector<std::shared_ptr<Behaviour>> behaviours = GetBehaviours();
        for (auto it = behaviours.begin(); it != behaviours.end(); )
        {
            auto &behaviour = *it;

            if (behaviour->IsActive())
            {
                ZoneNamedN(postUpdateZone, "Post-Update", true);
                ZoneTextV(postUpdateZone, behaviour->GetTypeName().data(), behaviour->GetTypeName().size());

                behaviour->PostUpdate();
            }

            ++it;
        }
    }

    PrepareRender();
}

void SceneManager::PrepareRender()
{
    ZoneScoped;

    // Prepare all active Scenes for rendering
    for (auto it = m_activeScenes.begin(); it != m_activeScenes.end(); )
    {
        auto scene = it->lock();

        if (!scene)
        {
            it = m_activeScenes.erase(it);
            continue;
        }

        scene->PrepareRender();

        ++it;
    }
}

std::vector<Droplet::Common::CameraRenderData> SceneManager::GetRenderData()
{
    ZoneScoped;

	using namespace Droplet::Graphics;
using namespace Droplet::Common;

    std::vector<CameraRenderData> renderViews{};
    
    for (auto it = m_activeScenes.begin(); it != m_activeScenes.end(); )
    {
        auto scene = it->lock();

		if (!scene)
		{
			it = m_activeScenes.erase(it);
			continue;
		}

        if (!scene->IsActive())
        {
            it = m_activeScenes.erase(it);
            continue;
        }
        
        auto &renderers = scene->m_renderers;

		for (std::shared_ptr<CameraComponent> &renderer : renderers)
		{
			if (!renderer)
			{
				continue;
			}

			renderViews.emplace_back(renderer->GetRenderData());
		}
        
        ++it;
    }

	// Sort the render views by priority (highest priority first)
	std::sort(renderViews.begin(), renderViews.end(), [](const CameraRenderData &a, const CameraRenderData &b) {
		return a.priority > b.priority;
	});

    return renderViews;
}

void SceneManager::PostRender()
{
    ZoneScoped;

	// Perform post-rendering operations for all active Scenes
    for (auto it = m_activeScenes.begin(); it != m_activeScenes.end(); )
    {
        auto scene = it->lock();

        if (!scene)
        {
            it = m_activeScenes.erase(it);
            continue;
        }

        scene->PostRender();

        ++it;
    }
}

void SceneManager::SerializeToFile(std::shared_ptr<Scene> p_scene, const std::string &p_filePath)
{
    ZoneScoped;

    SceneSerializer serializer;
    nlohmann::json json;

    json = serializer.SerializeScene(p_scene);
    Droplet::Core::JsonIO::Write(p_filePath, json);
}

void SceneManager::LoadFromFile(const std::string &p_filePath)
{
    ZoneScoped;

    nlohmann::json json = Droplet::Core::JsonIO::Read(p_filePath);
    std::shared_ptr<Scene> scene = CreateSceneFromJson(json);
    const std::string name = json["name"].get<std::string>();

    if (m_scenes.contains(name))
    {
        UnloadScene(name);
    }

    m_scenes[name] = scene;
}

void SceneManager::LoadFromFileAsync(const std::string &p_filePath, std::function<void(std::shared_ptr<Scene>)> p_onLoaded)
{
    ZoneScoped;

    ++m_pendingAsyncSceneLoads;

    ThreadPool::GetInstance().PushTask([this, p_filePath, onLoaded = std::move(p_onLoaded)]() mutable
    {
        AsyncSceneLoadResult result{ p_filePath, {}, nullptr, {}, std::move(onLoaded) };

        try
        {
            nlohmann::json json = Droplet::Core::JsonIO::Read(p_filePath);

            if (!json.contains("name"))
            {
                throw std::runtime_error("Scene json has no 'name' field");
            }

            result.sceneName = json["name"].get<std::string>();
            result.scene = CreateSceneFromJson(json);
        }
        catch (const std::exception &e)
        {
            result.error = e.what();
        }

        m_asyncSceneReadResults.Push(std::move(result));
    });
}

bool SceneManager::HasPendingAsyncLoads() const
{
    return m_pendingAsyncSceneLoads.load() > 0;
}

bool SceneManager::IsLoadingComplete() const
{
    if (HasPendingAsyncLoads())
    {
        return false;
    }

    for (const auto &[name, scene] : m_scenes)
    {
        if (!scene->AreResourcesLoaded())
        {
            return false;
        }
    }

    return true;
}

void SceneManager::FinalizeAsyncSceneLoads()
{
    ZoneScoped;

    AsyncSceneLoadResult result;
    while (m_asyncSceneReadResults.Pop(result))
    {
        --m_pendingAsyncSceneLoads;

        if (!result.error.empty())
        {
            std::cerr << "Failed to load Scene from '" << result.filePath << "': " << result.error << '\n';

            if (result.onLoaded)
            {
                result.onLoaded(nullptr);
            }
            continue;
        }

        if (m_scenes.contains(result.sceneName))
        {
            UnloadScene(result.sceneName);
        }

        m_scenes[result.sceneName] = result.scene;

        if (result.onLoaded)
        {
            result.onLoaded(result.scene);
        }
    }
}

std::shared_ptr<Scene> SceneManager::CreateSceneFromJson(const nlohmann::json &p_json)
{
    ZoneScoped;

    SceneSerializer serializer;

    if (!p_json.contains("name"))
    {
        throw std::runtime_error("Invalid json was sent to LoadFromFile");
    }

    std::shared_ptr<Scene> scene = SceneFactory::CreateScene(p_json["name"]);
    if (!scene)
    {
        throw std::runtime_error("Cannot load Scene "+ p_json["name"].get<std::string>());
    }
    scene->SetSceneManager(this);
    scene->Load();
    serializer.DeserializeScene(p_json, scene);

    return scene;
}


void SceneManager::SerializeNodeToFile(Node &p_node, const std::string &p_filePath)
{
    ZoneScoped;

    SceneSerializer serializer;
    nlohmann::json json;

    json = serializer.SerializeNode(p_node, true);
    Droplet::Core::JsonIO::Write(p_filePath, json);

    std::string relativePath = std::filesystem::relative(
        std::filesystem::absolute(p_filePath),
        std::filesystem::path(ROOT_PATH)
    ).string();

    p_node.SetPrefabPath(relativePath);
}

void SceneManager::SerializeBehaviours()
{
	nlohmann::json data;
    
	data["behaviours"] = nlohmann::json::array();
	for (const auto& behaviour : m_behaviours)
	{
		data["behaviours"].push_back(behaviour->Serialize());
	}
    
	if (!std::filesystem::exists(std::filesystem::path(BEHAVIOUR_SAVE_PATH)))
	{
		Core::JsonIO::CreateFile(std::string(BEHAVIOUR_SAVE_PATH));
	}

	try
	{
		Core::JsonIO::Write(std::string(BEHAVIOUR_SAVE_PATH), data);
	}
	catch (const std::runtime_error& e)
	{
		Debug::Log(Debug::LogType::Error, std::string("SceneManager: Failed to save behaviour data: ") + e.what());
	}
}

void SceneManager::DeserializeBehaviours()
{
	m_behaviours.clear();
    
	nlohmann::json data;
	try
	{
		data = Core::JsonIO::Read(std::string(BEHAVIOUR_SAVE_PATH));
	}
	catch (const std::runtime_error &e)
	{
		Debug::Log(Debug::LogType::Error, std::string("SceneManager: Failed to read saved behaviour data: ") + e.what());
		return;
	}
    
	if (data.contains("behaviours"))
	{
		for (const auto &entry : data["behaviours"])
		{
			if (entry.contains("type"))
			{
				std::string type = entry["type"];
				auto behaviour = BehaviourFactory::CreateBehaviour(type);
				if (!behaviour)
				{
					Debug::Log(Debug::LogType::Warning, "SceneManager: Failed to deserialize unknown behaviour type: "+ type);
					continue;
				}
				behaviour->SetSceneManager(this);
				m_behaviours.push_back(std::move(behaviour));   
				m_behaviours.back()->Deserialize(entry);
				m_behaviours.back()->Start();
			}
		}
	}
}

std::weak_ptr<Behaviour> SceneManager::StartBehaviour(const std::string &p_behaviourTypeName)
{
	std::shared_ptr<Behaviour> behaviour = std::move(BehaviourFactory::CreateBehaviour(p_behaviourTypeName));

    behaviour->SetSceneManager(this);
    {
        std::lock_guard<std::recursive_mutex> lock(m_behavioursMutex);
        m_behaviours.push_back(behaviour);
    }

    behaviour->Start();

	return behaviour;
}

void SceneManager::ShutdownBehaviour(std::weak_ptr<Behaviour> p_behaviour)
{
    auto behaviour = p_behaviour.lock();
    if (!behaviour)
    {
		return;
    }

    std::lock_guard<std::recursive_mutex> lock(m_behavioursMutex);
    auto it = std::find(m_behaviours.begin(), m_behaviours.end(), behaviour);
    if (it != m_behaviours.end())
    {
        m_behaviours.erase(it);
	}
}

std::vector<std::shared_ptr<Behaviour>> SceneManager::GetBehaviours() const
{
    std::lock_guard<std::recursive_mutex> lock(m_behavioursMutex);
    return m_behaviours;
}

std::weak_ptr<CameraComponent> SceneManager::GetMainCamera() const
{
	std::shared_ptr<CameraComponent> mainCamera = nullptr;
	for (const auto &sceneWeak : m_activeScenes)
	{
		auto scene = sceneWeak.lock();
		if (!scene)
		{
			continue;
		}

		const auto &cameraStack = scene->GetCameraStack();
		for (const auto &cameraWeak : cameraStack)
		{
			auto camera = cameraWeak.lock();
			if (!camera)
			{
				continue;
			}

			if (!mainCamera || camera->GetPriority() < mainCamera->GetPriority())
			{
				mainCamera = camera;
			}
		}
	}

	return mainCamera;
}
