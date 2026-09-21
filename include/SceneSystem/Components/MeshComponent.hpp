#pragma once

#include <SceneSystem/Component.hpp>
#include <string>

/// @brief Component that attaches a mesh to a Node.
/// 
/// an example of a MeshComponent could be a 3D model of a character, an object, or an environment element.
/// 
class MeshComponent : public Component
{
public:
    explicit MeshComponent(const std::string &p_meshPath);
    void Update(float p_deltaTime) override;

    /// TODO: Placeholder as the correct method is not yet inplemented
    const std::string &GetMeshPath() const;
       
private:
    std::string m_meshPath;
};