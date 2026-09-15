#include "SceneFactory.hpp"
#include "Scene.hpp"


std::shared_ptr<Scene> SceneFactory::CreateScene(const std::string& p_name)
{
    if (p_name == "Game")
    {
        return std::make_shared<Scene>("Game");
    }
	return nullptr;
}