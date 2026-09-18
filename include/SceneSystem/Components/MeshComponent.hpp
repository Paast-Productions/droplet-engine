#pragma once

#include "../Component.hpp"
#include <string>

/// @brief Component that attaches a mesh to a Node.
/// 
/// an example of a MeshComponent could be a 3D model of a character, an object, or an environment element.
/// 

class MeshComponent : public Component
{
public:
    explicit MeshComponent(std::string p_meshPath);

    void Initialize() override;
    void Update(float p_deltaTime) override;

    const std::string& GetMeshPath() const;
       
private:
    std::string m_meshPath;
};