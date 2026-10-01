#include <gtest/gtest.h>
#include "../include/resource/loaders/AssimpLoader.hpp"
#include <filesystem>
#include <json/json.hpp>
#include "resource/meta/MetaUtils.hpp"

namespace fs = std::filesystem;
using namespace Droplet;
using json = nlohmann::json;

class AssimpLoaderTest : public ::testing::Test
{
protected:
	fs::path m_testAssetPath;

	void SetUp() override
	{
		m_testAssetPath = fs::path(TEST_ASSET_DIR) / "Meshes" / "EmptyMesh.obj";
		ASSERT_TRUE(fs::exists(m_testAssetPath)) << "File does not exist: " << m_testAssetPath;
		m_testAssetPath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFishWithNoMesh.fbx";
		ASSERT_TRUE(fs::exists(m_testAssetPath)) << "File does not exist: " << m_testAssetPath;
		m_testAssetPath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFishWithNoAnim.fbx";
		ASSERT_TRUE(fs::exists(m_testAssetPath)) << "File does not exist: " << m_testAssetPath;
		m_testAssetPath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFish.fbx";
		ASSERT_TRUE(fs::exists(m_testAssetPath)) << "File does not exist: " << m_testAssetPath;
		m_testAssetPath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFish.glb";
		ASSERT_TRUE(fs::exists(m_testAssetPath)) << "File does not exist: " << m_testAssetPath;
	}
};

// ==========
// Mesh tests
// ==========

TEST_F(AssimpLoaderTest, LoadNonExistingMesh)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Mesh,
		"", "DoesNotExist.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "DoesNotExist.fbx";
	EXPECT_THROW(auto temp = AssimpLoader::LoadMesh(filePath, metaEntry.loadSettings), std::runtime_error);
}

TEST_F(AssimpLoaderTest, LoadMeshWithNoMeshData)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Mesh,
		"", "CorruptedWoodFishWithNoMesh.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFishWithNoMesh.fbx";
	EXPECT_THROW(auto temp = AssimpLoader::LoadMesh(filePath, metaEntry.loadSettings), std::runtime_error);
}

TEST_F(AssimpLoaderTest, LoadMeshFbx)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Mesh,
		"", "CorruptedWoodFish.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFish.fbx";
	EXPECT_TRUE(AssimpLoader::LoadMesh(filePath, metaEntry.loadSettings) != nullptr);
}

TEST_F(AssimpLoaderTest, LoadMeshGlb)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Mesh,
		"", "CorruptedWoodFish.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFish.glb";
	EXPECT_TRUE(AssimpLoader::LoadMesh(filePath, metaEntry.loadSettings) != nullptr);
}

// ==================
// Skinned mesh tests
// ==================

TEST_F(AssimpLoaderTest, LoadNonExistingSkinnedMesh)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::SkinnedMesh,
		"", "DoesNotExist.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "DoesNotExist.fbx";
	EXPECT_THROW(auto temp = AssimpLoader::LoadSkinnedMesh(filePath, metaEntry.loadSettings), std::runtime_error);
}

TEST_F(AssimpLoaderTest, LoadSkinnedMeshWithNoMeshData)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::SkinnedMesh,
		"", "CorruptedWoodFishWithNoMesh.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFishWithNoMesh.fbx";
	EXPECT_THROW(auto temp = AssimpLoader::LoadSkinnedMesh(filePath, metaEntry.loadSettings), std::runtime_error);
}

TEST_F(AssimpLoaderTest, LoadSkinnedMeshWithNoAnimData)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::SkinnedMesh,
		"", "CorruptedWoodFishWithNoAnim.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFishWithNoAnim.fbx";
	EXPECT_THROW(auto temp = AssimpLoader::LoadSkinnedMesh(filePath, metaEntry.loadSettings), std::runtime_error);
}

TEST_F(AssimpLoaderTest, LoadSkinnedMeshFbx)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::SkinnedMesh,
		"", "CorruptedWoodFish.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFish.fbx";
	EXPECT_TRUE(AssimpLoader::LoadSkinnedMesh(filePath, metaEntry.loadSettings) != nullptr);
}

TEST_F(AssimpLoaderTest, LoadSkinnedMeshGlb)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::SkinnedMesh,
		"", "CorruptedWoodFish.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFish.fbx";
	EXPECT_TRUE(AssimpLoader::LoadSkinnedMesh(filePath, metaEntry.loadSettings) != nullptr);
}

// ===============
// Animation tests
// ===============

TEST_F(AssimpLoaderTest, LoadNonExistingAnimation)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Animation,
		"", "DoesNotExist.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "DoesNotExist.fbx";
	EXPECT_THROW(auto temp = AssimpLoader::LoadAnimation(filePath, metaEntry.loadSettings), std::runtime_error);
}

TEST_F(AssimpLoaderTest, LoadAnimationWithNoMeshData)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Animation,
		"Fish|ArmatureAction", "CorruptedWoodFishWithNoMesh.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFishWithNoMesh.fbx";
	EXPECT_TRUE( AssimpLoader::LoadAnimation(filePath, metaEntry.loadSettings) != nullptr);
}

TEST_F(AssimpLoaderTest, LoadAnimationWithNoAnimData)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Animation,
		"Fish|ArmatureAction", "CorruptedWoodFishWithNoAnim.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFishWithNoAnim.fbx";
	EXPECT_THROW(auto temp = AssimpLoader::LoadAnimation(filePath, metaEntry.loadSettings), std::runtime_error);
}

TEST_F(AssimpLoaderTest, LoadAnimationFbx)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Animation,
		"Fish|ArmatureAction", "CorruptedWoodFish.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFish.fbx";
	EXPECT_TRUE(AssimpLoader::LoadAnimation(filePath, metaEntry.loadSettings) != nullptr);
}

TEST_F(AssimpLoaderTest, LoadAnimationGlb)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Animation,
		"ArmatureAction", "CorruptedWoodFish.glb");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFish.glb";
	EXPECT_TRUE(AssimpLoader::LoadAnimation(filePath, metaEntry.loadSettings) != nullptr);
}