#include "SceneFactory.hpp"
#include "Scene.hpp"

std::shared_ptr<Scene> SceneFactory::CreateScene(const std::string& p_name)
{
	return std::make_shared<Scene>(p_name);
}