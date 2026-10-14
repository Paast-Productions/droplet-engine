#include "resource/meta/MetaUtils.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using namespace Droplet;

class MetaUtilsTest : public ::testing::Test
{
protected:
    fs::path m_tempMetaPath;
    fs::path m_malformedJsonPath;

    void SetUp() override
    {
        // Use the OS temp directory so we don't bloat the engine folder
        fs::path tempDir = fs::temp_directory_path();
        
        m_tempMetaPath = tempDir / "Test_RoundTrip.meta";
        m_malformedJsonPath = tempDir / "Test_Malformed.meta";

        // Create a malformed JSON file for error testing
        std::ofstream malformed(m_malformedJsonPath);
        malformed << "{ this is not valid json }";
        malformed.close();
    }

    void TearDown() override
    {
        if (fs::exists(m_tempMetaPath)) fs::remove(m_tempMetaPath);
        if (fs::exists(m_malformedJsonPath)) fs::remove(m_malformedJsonPath);
    }
};

TEST_F(MetaUtilsTest, ResourceLoadFlagsBitwise)
{
    ResourceLoadFlag both = ResourceLoadFlag::LoadCPU | ResourceLoadFlag::LoadGPU;
    EXPECT_EQ(both, ResourceLoadFlag::LoadBoth);

    EXPECT_TRUE(HasFlag(ResourceLoadFlag::LoadBoth, ResourceLoadFlag::LoadCPU));
    EXPECT_TRUE(HasFlag(ResourceLoadFlag::LoadBoth, ResourceLoadFlag::LoadGPU));
    EXPECT_FALSE(HasFlag(ResourceLoadFlag::LoadCPU, ResourceLoadFlag::LoadGPU));
}

TEST_F(MetaUtilsTest, EvaluateShaderTypeFromPath)
{
    // Exact match
    EXPECT_EQ(MetaUtils::EvaluateShaderTypeFromPath("vs_main.slang"), ShaderResource::ShaderType::Vertex);
    EXPECT_EQ(MetaUtils::EvaluateShaderTypeFromPath("fs_main.slang"), ShaderResource::ShaderType::Fragment);
    
    // Case insensitivity (must convert to lowercase internally)
    EXPECT_EQ(MetaUtils::EvaluateShaderTypeFromPath("CS_Compute.slang"), ShaderResource::ShaderType::Compute);
    
    // Path extraction (should ignore folders)
    EXPECT_EQ(MetaUtils::EvaluateShaderTypeFromPath("Assets/Shaders/tes_hull.slang"), ShaderResource::ShaderType::TessellationEvaluation);
    
    // Default fallback
    EXPECT_EQ(MetaUtils::EvaluateShaderTypeFromPath("unknown_shader.slang"), ShaderResource::ShaderType::Vertex);
}

TEST_F(MetaUtilsTest, GenerateDefaultMetaEntry)
{
    // Texture defaults
    MetaEntry texEntry = MetaUtils::GenerateDefaultMetaEntry(ResourceType::Texture2D, "MyTex", "path.png");
    EXPECT_NE(texEntry.guid, C_INVALID_GUID);
    EXPECT_EQ(texEntry.loadFlags, ResourceLoadFlag::LoadBoth);
    EXPECT_TRUE(texEntry.loadSettings.contains(MetaLoadSettings::C_GENERATE_MIPMAPS.key));

    // Mesh defaults
    MetaEntry meshEntry = MetaUtils::GenerateDefaultMetaEntry(ResourceType::Mesh, "MyMesh", "path.fbx");
    EXPECT_EQ(meshEntry.loadFlags, ResourceLoadFlag::LoadBoth);
    EXPECT_TRUE(meshEntry.loadSettings.contains(MetaLoadSettings::C_GENERATE_NORMALS.key));

    // Explicit Overrides
    nlohmann::json explicitSettings = { {MetaLoadSettings::C_GENERATE_MIPMAPS.key, false}, {"custom_field", 42} };
    MetaEntry overrideEntry = MetaUtils::GenerateDefaultMetaEntry(
        ResourceType::Texture2D, "MyTex", "path.png", explicitSettings);
    
    EXPECT_EQ(overrideEntry.loadSettings[MetaLoadSettings::C_GENERATE_MIPMAPS.key], false); // Overwritten
    EXPECT_EQ(overrideEntry.loadSettings["custom_field"], 42);        // Appended
}

TEST_F(MetaUtilsTest, WriteAndReadRoundTrip)
{
    // Create dummy entries
    std::vector<MetaEntry> entriesToWrite;
    
    MetaEntry entry1 = MetaUtils::GenerateDefaultMetaEntry(ResourceType::Texture2D, "Tex", "Assets/tex.png");
    entry1.dependencies = { 1010101, 2020202 }; // Fake GUIDs

    MetaEntry entry2 = MetaUtils::GenerateDefaultMetaEntry(ResourceType::Shader, "Shd", "Assets/vs_shd.slang");

    entriesToWrite.push_back(entry1);
    entriesToWrite.push_back(entry2);

    // Write to disk
    ASSERT_TRUE(MetaUtils::Write(m_tempMetaPath, entriesToWrite));
    ASSERT_TRUE(fs::exists(m_tempMetaPath));

    // Read back from disk
    std::vector<MetaEntry> entriesRead;
    ASSERT_TRUE(MetaUtils::Read(m_tempMetaPath, entriesRead));

    // Verify data survived intact
    ASSERT_EQ(entriesRead.size(), 2);
    
    EXPECT_EQ(entriesRead[0].guid, entry1.guid);
    EXPECT_EQ(entriesRead[0].type, entry1.type);
    EXPECT_EQ(entriesRead[0].loadFlags, entry1.loadFlags);
    
    ASSERT_EQ(entriesRead[0].dependencies.size(), 2);
    EXPECT_EQ(entriesRead[0].dependencies[0], 1010101);
    
    // Check JSON parse
    EXPECT_EQ(entriesRead[0].loadSettings[MetaLoadSettings::C_GENERATE_MIPMAPS.key], entry1.loadSettings[MetaLoadSettings::C_GENERATE_MIPMAPS.key]);
}

TEST_F(MetaUtilsTest, ReadNonExistentFile)
{
    std::vector<MetaEntry> entries;
    fs::path missingFile = fs::temp_directory_path() / "DoesNotExist123.meta";
    
    EXPECT_FALSE(MetaUtils::Read(missingFile, entries));
}

TEST_F(MetaUtilsTest, ReadMalformedJson)
{
    std::vector<MetaEntry> entries;
    EXPECT_FALSE(MetaUtils::Read(m_malformedJsonPath, entries));
}