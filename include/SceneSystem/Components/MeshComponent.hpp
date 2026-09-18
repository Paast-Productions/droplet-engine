#pragma once

#include "../Component.hpp"
#include <string>

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