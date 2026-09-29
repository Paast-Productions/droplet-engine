#include <gtest/gtest.h>
#include "../include/resource/loaders/TextureLoader.hpp"
#include "../include/resource/types/Texture2DResource.hpp"

#include <filesystem>

namespace fs = std::filesystem;
using namespace Droplet;
class TextureLoaderTest : public ::testing::Test
{
protected:

	std::shared_ptr<TextureLoader> m_textureLoader;

	fs::path m_testAssetPath;
	
	void SetUp() override
	{
		m_testAssetPath = fs::path(TEST_ASSET_DIR) / "Textures" / "Test_BlackPixel.ktx";

		ASSERT_TRUE(fs::exists(m_testAssetPath)) << "Texture does not exist: " << m_testAssetPath;

		m_textureLoader = std::make_shared<TextureLoader>();

	}
};

TEST_F(TextureLoaderTest, Construct)
{
	EXPECT_NE(m_textureLoader, nullptr);
}

TEST_F(TextureLoaderTest, LoadValidPath)
{
	std::shared_ptr<Texture2DResource> texture = m_textureLoader->Load(m_testAssetPath.string());

	ASSERT_NE(texture, nullptr);

	EXPECT_EQ(texture->GetWidth(), 1);
	EXPECT_EQ(texture->GetHeight(), 1);
	EXPECT_EQ(texture->GetMipLevels(), 1);

}

TEST_F(TextureLoaderTest, LoadNonExistingFile)
{
	fs::path nonExistingPath = fs::path(TEST_ASSET_DIR) / "Textures" / "DoesNotExist.ktx";
	EXPECT_THROW( m_textureLoader->Load(nonExistingPath.string()), std::runtime_error);

}

TEST_F(TextureLoaderTest, LoadInvalidFile)
{
	fs::path invalidPath = fs::path(TEST_ASSET_DIR) / "Textures" / "InvalidTexture.txt";
	EXPECT_THROW(m_textureLoader->Load(invalidPath.string()), std::runtime_error);

}

TEST_F(TextureLoaderTest, LoadEmptyFile)
{
	fs::path emptyPath = fs::path(TEST_ASSET_DIR) / "Textures" / "EmptyTexture.ktx";
	EXPECT_THROW(m_textureLoader->Load(emptyPath.string()), std::runtime_error);
}

TEST_F(TextureLoaderTest, LoadAndConvertPNGFile)
{
	fs::path pngPath = fs::path(TEST_ASSET_DIR) / "Textures" / "Test_BlackPixel.png";
	std::shared_ptr<Texture2DResource> texture = m_textureLoader->Load(pngPath.string());

	ASSERT_NE(texture, nullptr);
	EXPECT_EQ(texture->GetWidth(), 1);
	EXPECT_EQ(texture->GetHeight(), 1);
	
}