#include "resource/ResourceRegistry.hpp"
#include "resource/meta/MetaUtils.hpp"

#include <gtest/gtest.h>
#include <filesystem>

namespace fs = std::filesystem;
using namespace Droplet;

TEST(ResourceRegistryTest, RegisterAndGetByGUID)
{
    ResourceRegistry registry;
    
    MetaEntry texEntry;
    texEntry.guid = 100;
    texEntry.type = ResourceType::Texture2D;
    texEntry.name = "MyTexture";
    
    registry.RegisterMetaEntry("Assets/Tex.png", texEntry);
    
    // Valid GUID should return the entry
    MetaEntry retrieved = registry.GetResourceMetaData(100);
    EXPECT_EQ(retrieved.type, ResourceType::Texture2D);
    EXPECT_EQ(retrieved.name, "MyTexture");
    
    // Invalid GUID should throw
    EXPECT_THROW(registry.GetResourceMetaData(999), std::runtime_error);
}

TEST(ResourceRegistryTest, RegisterAndGetByAssetPath)
{
    ResourceRegistry registry;
    
    // Simulate an FBX file that contains TWO resources (a mesh and an animation)
    MetaEntry meshEntry; meshEntry.guid = 1; meshEntry.type = ResourceType::Mesh;
    MetaEntry animEntry; animEntry.guid = 2; animEntry.type = ResourceType::Animation;
    
    registry.RegisterMetaEntry("Assets/Hero.fbx", meshEntry);
    registry.RegisterMetaEntry("Assets/Hero.fbx", animEntry);
    
    std::vector<MetaEntry> assetResources;
    
    // Valid asset path
    bool found = registry.GetCachedMetaDataForAsset("Assets/Hero.fbx", assetResources);
    ASSERT_TRUE(found);
    ASSERT_EQ(assetResources.size(), 2);
    
    // Invalid asset path
    std::vector<MetaEntry> emptyResources;
    EXPECT_FALSE(registry.GetCachedMetaDataForAsset("Assets/Missing.fbx", emptyResources));
}

TEST(ResourceRegistryTest, GetEntriesFilteredByType)
{
    ResourceRegistry registry;
    
    MetaEntry tex1; tex1.guid = 1; tex1.type = ResourceType::Texture2D;
    MetaEntry tex2; tex2.guid = 2; tex2.type = ResourceType::Texture2D;
    MetaEntry mesh1; mesh1.guid = 3; mesh1.type = ResourceType::Mesh;
    
    registry.RegisterMetaEntry("Tex1.png", tex1);
    registry.RegisterMetaEntry("Tex2.png", tex2);
    registry.RegisterMetaEntry("Mesh1.fbx", mesh1);
    
    // Test No Filter (All entries)
    auto allEntries = registry.GetEntries();
    EXPECT_EQ(allEntries.size(), 3);
    
    // Test Type Filter (Only Textures)
    auto texEntries = registry.GetEntries(ResourceType::Texture2D);
    EXPECT_EQ(texEntries.size(), 2);
    
    // Test Type Filter (Only Meshes)
    auto meshEntries = registry.GetEntries(ResourceType::Mesh);
    EXPECT_EQ(meshEntries.size(), 1);
    
    // Test Type Filter (Empty result)
    auto shaderEntries = registry.GetEntries(ResourceType::Shader);
    EXPECT_TRUE(shaderEntries.empty());
}

class ResourceRegistryScanTest : public ::testing::Test
{
protected:
    fs::path m_scanDir;

    void SetUp() override
    {
        m_scanDir = fs::path(TEST_ASSET_DIR);
        fs::path shaderMeta = m_scanDir / "Shaders" / "Test_Valid.slang.meta";
        fs::path textureMeta = m_scanDir / "Textures" / "Test_BlackPixel.png.meta";

        ASSERT_TRUE(fs::exists(shaderMeta)) << "Missing meta file: " << shaderMeta;
        ASSERT_TRUE(fs::exists(textureMeta)) << "Missing meta file: " << textureMeta;
    }
};

TEST_F(ResourceRegistryScanTest, ScanDirectoryValid)
{
    ResourceRegistry registry;
    
    // Perform the scan on the entire Asset directory
    ASSERT_NO_THROW(registry.ScanDirectory(m_scanDir));
    
    // Verify the registry found and parsed your specific hardcoded files
    EXPECT_NO_THROW(registry.GetResourceMetaData(500));
    EXPECT_NO_THROW(registry.GetResourceMetaData(600));

    // Ensure it parsed your JSON correctly
    MetaEntry texEntry = registry.GetResourceMetaData(500);
    EXPECT_EQ(texEntry.relAssetPath, "Textures/Test_BlackPixel.png");
    EXPECT_EQ(static_cast<uint8_t>(texEntry.type), static_cast<uint8_t>(ResourceType::Texture2D)); 
    EXPECT_EQ(texEntry.loadSettings["generate_mipmaps"], false);

    MetaEntry shaderEntry = registry.GetResourceMetaData(600);
    EXPECT_EQ(shaderEntry.relAssetPath, "Shaders/Test_Valid.slang");
    EXPECT_EQ(static_cast<uint8_t>(shaderEntry.type), static_cast<uint8_t>(ResourceType::Shader));
}

TEST_F(ResourceRegistryScanTest, ScanDirectoryInvalidThrows)
{
    ResourceRegistry registry;
    fs::path invalidDir = m_scanDir / "ThisFolderDoesNotExist";
    
    EXPECT_THROW(registry.ScanDirectory(invalidDir), std::runtime_error);
}
