#pragma once

#include <memory>
#include <string>

class SceneNode;

class scene
{
public:
    explicit Scene(const std::string& p_name);
    virtual ~Scene() = default;

    virtual void Load();
    virtual void Unload();

    virtual void Update(float p_deltaTime);
    virtual void Render();

	std::unique_ptr<SceneNode> GetRoot() const;

    const std::string& GetName() const;

    bool IsLoaded() const;
    bool IsActive() const;

    void SetActive(bool active);

private:
    std::string m_name;
    std::shared_ptr<SceneNode> m_root;

    bool m_loaded = false;
    bool m_active = false;

};