#pragma once

#include "asset/meta/MetaData.hpp"

#include <filesystem>

namespace Droplet::MetaSerializer
{
    // --- Default Import Settings ---
    
    // Models
    static constexpr float C_MODEL_DEFAULT_SCALE = 1.0f; // The default scale in a 
    static constexpr bool C_MODEL_DEFAULT_IMPORT_ANIMATIONS = false;
    
    // Textures
    // static constexpr bool C_TEXTURE_DEFAULT_GENERATE_MIPMAPS = false;
    
    /// @brief Reads and parses a .meta file.
    /// @param p_metaFilePath The path to the .meta file on the disk.
    /// @param p_metaData The metadata struct to be populated.
    /// @return True if the file was successfully read, otherwise false.
    bool Read(const std::filesystem::path &p_metaFilePath, ResourceMetaData p_metaData);

    /// @brief Writes a metadata to a .meta file.
    /// @param p_metaFilePath The path to the .meta file.
    /// @param p_metaData Struct containing the metadata to be written to the .meta file.
    /// @return True if the metadata was successfully written to the file, otherwise false.
    bool Write(const std::filesystem::path &p_metaFilePath, const ResourceMetaData &p_metaData);

    /// @brief Retrieves the asset extension for a specified file.
    /// @param p_assetPath The path to the asset file.
    /// @return The extension. Empty string if the file does not exist.
    std::string GetLowercaseExtension(const std::filesystem::path &p_assetPath);

    /// @brief Generates a meta file with a newly generated GUID and default values for every other field.
    /// @param p_assetPath The path to the asset file.
    /// @param p_metaData The metadata struct to be populated.
    /// @return True if the file was successfully generated, otherwise false.
    bool GenerateDefaultMetaFile(const std::filesystem::path &p_assetPath, ResourceMetaData &p_metaData);
}