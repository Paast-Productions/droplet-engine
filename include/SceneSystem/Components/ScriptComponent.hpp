#pragma once

#include "Component.hpp"

#include <string>

/// @brief Component that attaches a Lua script to a Node.
///
/// A ScriptComponent stores the path to a Lua script and provides
/// the interface required to initialize and update the script.
///
/// Multiple ScriptComponents can be attached to the same Node,
/// allowing a Node to have multiple Lua scripts with different
/// responsibilities.
class ScriptComponent : public Component
{
public:

    /// @brief Constructs a ScriptComponent with the specified script path.
    ///
    /// @param p_scriptPath Path to the Lua script.
    explicit ScriptComponent(std::string p_scriptPath);

    /// @brief Initializes the script component.
    ///
    /// Loads and initializes the associated Lua script.
    /// The actual Lua integration can be implemented separately.
    void Initialize() override;

    /// @brief Updates the Lua script.
    ///
    /// Called once per update while the owning Node is active.
    ///
    /// @param p_deltaTime Time elapsed since the previous update, in seconds.
    void Update(float p_deltaTime) override;

    /// @brief Gets the path to the Lua script.
    ///
    /// @return A constant reference to the script path.
    const std::string& GetScriptPath() const;

private:

    std::string m_scriptPath;
};