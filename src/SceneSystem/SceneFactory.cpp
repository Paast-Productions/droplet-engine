#include "SceneFactory.hpp"
#include "Scene.hpp"
#include <tracy/public/tracy/Tracy.hpp>

using namespace Droplet::Scene;

std::shared_ptr<Scene> SceneFactory::CreateScene(const std::string &p_name)
{
	ZoneScoped;

	return std::make_shared<Scene>(p_name);
}