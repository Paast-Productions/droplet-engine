#include "resource/loaders/GliLoader.hpp"

#include <gtest/gtest.h>
#include <json/json.hpp>
#include <filesystem>
#include <gli/texture.hpp>

namespace fs = std::filesystem;
using namespace Droplet;

class GliLoaderTest : public ::testing::Test
{
protected:
	fs::path m_validKtxPath;
    fs::path m_validPngPath;
    fs::path m_emptyKtxPath;
    fs::path m_nonTexturePath;
    fs::path m_invalidPath;
    
    nlohmann::json m_defaultLoadSettings;
	
	void SetUp() override
	{
	    fs::path baseDir(TEST_ASSET_DIR);
		m_validKtxPath = baseDir / "Textures" / "Test_BlackPixel.ktx";
	    m_validPngPath = baseDir / "Textures" / "Test_BlackPixel.png";
	    m_emptyKtxPath = baseDir / "Textures" / "EmptyTexture.ktx";
	    m_nonTexturePath = baseDir / "Textures" / "NonTexture.txt";
	    m_invalidPath = baseDir / "Textures" / "ThisPathDoesNotExist.ktx";
	    
		ASSERT_TRUE(fs::exists(m_validKtxPath)) << "Missing test asset: " << m_validKtxPath;
		ASSERT_TRUE(fs::exists(m_validPngPath)) << "Missing test asset: " << m_validPngPath;
	    
	    m_defaultLoadSettings = nlohmann::json::object();
	}
};

TEST_F(GliLoaderTest, ListAssetResources)
{
	auto resources = GliLoader::ListAssetResources(m_validKtxPath);
    
    ASSERT_EQ(resources.size(), 1);
    EXPECT_EQ(resources[0].first, ResourceType::Texture2D);
}

TEST_F(GliLoaderTest, Load2DValidKtx)
{
	std::unique_ptr<Texture2DResource> texture2d = GliLoader::LoadTexture2D(m_validKtxPath, m_defaultLoadSettings);

	ASSERT_NE(texture2d, nullptr);
	EXPECT_EQ(texture2d->GetWidth(), 1);
	EXPECT_EQ(texture2d->GetHeight(), 1);
	// EXPECT_EQ(texture2d->GetMipLevels(), 1); // Not implemented really
}

TEST_F(GliLoaderTest, Load2DValidKtxPng)
{
    std::unique_ptr<Texture2DResource> texture2d = GliLoader::LoadTexture2D(m_validPngPath, m_defaultLoadSettings);
    
    ASSERT_NE(texture2d, nullptr);
    EXPECT_EQ(texture2d->GetWidth(), 1);
    EXPECT_EQ(texture2d->GetHeight(), 1);
    // EXPECT_EQ(texture2d->GetMipLevels(), 1); // Should we test this here?
}

TEST_F(GliLoaderTest, Load2DEmptyKtx)
{
    EXPECT_THROW(GliLoader::LoadTexture2D(m_emptyKtxPath, m_defaultLoadSettings), std::runtime_error);
}

TEST_F(GliLoaderTest, Load2DNonTextureFile)
{
    
    EXPECT_THROW(GliLoader::LoadTexture2D(m_nonTexturePath, m_defaultLoadSettings), std::runtime_error);
}

TEST_F(GliLoaderTest, Load2DInvalidPath)
{
    EXPECT_THROW(GliLoader::LoadTexture2D(m_invalidPath, m_defaultLoadSettings), std::runtime_error);
}