#include "Behaviour.hpp"

using namespace Droplet::Scene;

void Behaviour::SetSceneManager(SceneManager *sceneManager)
{
	m_sceneManager = sceneManager;
}

nlohmann::json Behaviour::Serialize()
{
	nlohmann::json behJson;

	behJson["type"] = GetTypeName();

	behJson["data"] = SerializeImpl();

	return SerializeImpl();
}

void Behaviour::Deserialize(const nlohmann::json p_behJson)
{
	const nlohmann::json dataJson = p_behJson["data"];

	DeserializeImpl(dataJson);
}