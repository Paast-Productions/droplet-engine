#pragma once

#include <memory>
#include <string>

class Scene;

class SceneFactory
{
public:
    static std::shared_ptr<Scene> CreateScene(const std::string& p_name);
};