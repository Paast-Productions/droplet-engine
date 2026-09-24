#pragma once

#include "resource/meta/MetaData.hpp"
#include "resource/types/ShaderResource.hpp"

#include <filesystem>

namespace Droplet::MetaUtils
{
    // --- Default Import Settings --- TODO: Add defualts for all flags here
    
    // Meshes
    static constexpr bool C_MESH_DEFAULT_IMPORT_GENERATE_NORMALS = true;
    static constexpr bool C_MESH_DEFAULT_IMPORT_JOIN_IDENTICAL_VERTICES = true;
    static constexpr bool C_MESH_DEFAULT_IMPORT_TRIANGULATE = true;
    
    // Textures
    static constexpr bool C_TEXTURE_DEFAULT_GENERATE_MIPMAPS = false;
    
    /// @brief Reads and parses a .meta file.
    /// @param p_metaFilePath The path to the .meta file on the disk.
    /// @param p_metaData The metadata struct to be populated.
    /// @return True if the file was successfully read, otherwise false.
    bool Read(const std::filesystem::path &p_metaFilePath, std::vector<MetaEntry> &p_metaData);

    /// @brief Writes a metadata to a .meta file.
    /// @param p_metaFilePath The path to the .meta file.
    /// @param p_metaData Struct containing the metadata to be written to the .meta file.
    /// @return True if the metadata was successfully written to the file, otherwise false.
    bool Write(const std::filesystem::path &p_metaFilePath, std::vector<MetaEntry> &p_metaData);

    /// @brief Determines the shader type based on its name.
    /// @param p_shaderPath The path to the shader file.
    /// @return The shader type.
    [[nodiscard]] ShaderResource::ShaderType EvaluateShaderTypeFromPath(const std::filesystem::path &p_shaderPath);

    /// @brief Generates a new meta entry with a unique GUID and type-specific default values.
    /// @param p_type The type of resource the entry is for.
    /// @param p_name The name of the resource within the asset.
    /// @param p_assetPath The path to the asset file the resource is stored within.
    /// @return A generated meta entry.
    [[nodiscard]] MetaEntry GenerateDefaultMetaEntry(ResourceType p_type, const std::string &p_name, const std::string &p_assetPath);
}