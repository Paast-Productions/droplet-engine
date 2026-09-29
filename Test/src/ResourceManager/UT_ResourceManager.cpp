#include <gtest/gtest.h>
#include <filesystem>

#include "../include/resource/ResourceManager.hpp"
#include "../include/resource/types/Texture2DResource.hpp"
#include "../include/resource/meta/MetaUtils.hpp"
#include "../include/resource/GUID.hpp"

#include <chrono>
#include <thread>

namespace fs = std::filesystem;
using namespace Droplet;
class ResourceManagerTest : public ::testing::Test
{
protected:

	std::shared_ptr<ResourceManager> m_resourceManager;

	GUID m_textureGuid;
	fs::path m_testAssetPath;

	bool WaitForResource(
		GUID p_guid,
		std::chrono::milliseconds p_timeout = std::chrono::seconds(1))
	{
		auto start = std::chrono::steady_clock::now();

		while (std::chrono::steady_clock::now() - start < p_timeout)
		{
			auto state = m_resourceManager->GetState(p_guid);

			if (state == ResourceState::ReadyAsync)
				return true;

			if (state == ResourceState::Failed)
				return false;

			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		return false;
	}

	void SetUp() override
	{
		m_testAssetPath = fs::path(TEST_ASSET_DIR) / "Textures" / "Test_BlackPixel.ktx";

		std::vector<MetaEntry> metadata;

		ASSERT_TRUE(fs::exists(m_testAssetPath)) << "Texture does not exist: " << m_testAssetPath;

		fs::path metaPath = m_testAssetPath;
		metaPath += ".meta";

		ASSERT_TRUE(fs::exists(metaPath));

		ASSERT_TRUE(MetaUtils::Read(metaPath, metadata));
		ASSERT_FALSE(metadata.empty());

		m_textureGuid = metadata[0].guid;

		m_resourceManager = std::make_shared<ResourceManager>();

		m_resourceManager->Initialize(TEST_ASSET_DIR);
	}

};

TEST_F(ResourceManagerTest, Construct)
{
	EXPECT_NE(m_resourceManager, nullptr);
}

TEST_F(ResourceManagerTest, LoadInvalidGUID)
{
	GUID invalidGuid = C_INVALID_GUID;

	auto hanlde = m_resourceManager->LoadResource<Texture2DResource>(invalidGuid);

	EXPECT_FALSE(hanlde.IsValid());
}

TEST_F(ResourceManagerTest, LoadResourceValidGUIDReturnValidHandle)
{
	auto handle = m_resourceManager->LoadResource<Texture2DResource>(m_textureGuid);

	EXPECT_TRUE(handle.IsValid());
}

TEST_F(ResourceManagerTest, LoadTextureTwice)
{
	auto handle1 = m_resourceManager->LoadResource<Texture2DResource>(m_textureGuid);

	auto handle2 = m_resourceManager->LoadResource<Texture2DResource>(m_textureGuid);

	EXPECT_TRUE(handle1.IsValid());
	EXPECT_TRUE(handle2.IsValid());

	EXPECT_EQ(handle1.GetGUID(), handle2.GetGUID());
	
}

TEST_F(ResourceManagerTest, CopyingHandleRefCount)
{
	auto handle = m_resourceManager->LoadResource<Texture2DResource>(m_textureGuid);

	EXPECT_EQ(handle.GetRef(), 1);

	{
		auto handle2 = handle;
		EXPECT_EQ(handle.GetRef(), 2);
		EXPECT_EQ(handle2.GetRef(), 2);
	}

	EXPECT_EQ(handle.GetRef(), 1);
}

TEST_F(ResourceManagerTest, InvalidGUIDState)
{
	EXPECT_EQ(
		m_resourceManager->GetState(C_INVALID_GUID),
		ResourceState::Unloaded
	);
}

TEST_F(ResourceManagerTest, LoadedResourceState)
{
	auto handle = m_resourceManager->LoadResource<Texture2DResource>(m_textureGuid);
	ASSERT_TRUE(handle.IsValid());

	ASSERT_TRUE(WaitForResource(m_textureGuid));

	EXPECT_EQ(
		m_resourceManager->GetState(m_textureGuid),
		ResourceState::ReadyAsync
	);
}

TEST_F(ResourceManagerTest, GetResourceBeforeReady)
{

	auto handle = m_resourceManager->LoadResource<Texture2DResource>(m_textureGuid);

	ASSERT_TRUE(handle.IsValid());
	
	ASSERT_TRUE(WaitForResource(m_textureGuid));
	auto resource = m_resourceManager->GetResource<Texture2DResource>(m_textureGuid);

	EXPECT_EQ(resource, nullptr);
}
