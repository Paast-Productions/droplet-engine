#include <gtest/gtest.h>
#include "resource/io/AssimpFormat.hpp"
#include "resource/meta/MetaUtils.hpp"
#include <filesystem>
#include <json/json.hpp>

namespace fs = std::filesystem;
using namespace Droplet;
using json = nlohmann::json;

class AssimpFormatTest : public ::testing::Test
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
		m_testAssetPath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFishMaterial.fbx";
		ASSERT_TRUE(fs::exists(m_testAssetPath)) << "File does not exist: " << m_testAssetPath;
	}
};

// ==========
// Mesh tests
// ==========

TEST_F(AssimpFormatTest, LoadNonExistingMesh)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Mesh,
		"", "DoesNotExist.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "EmptyMesh.fbx";
	EXPECT_THROW(auto temp = IO::AssimpFormat::LoadMesh(filePath, metaEntry.loadSettings), std::runtime_error);
}

TEST_F(AssimpFormatTest, LoadMeshWithNoMeshData)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Mesh,
		"", "CorruptedWoodFishWithNoMesh.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFishWithNoMesh.fbx";
	EXPECT_THROW(auto temp = IO::AssimpFormat::LoadMesh(filePath, metaEntry.loadSettings), std::runtime_error);
}

TEST_F(AssimpFormatTest, LoadMeshFbx)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Mesh,
		"", "CorruptedWoodFish.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFish.fbx";
	EXPECT_TRUE(IO::AssimpFormat::LoadMesh(filePath, metaEntry.loadSettings) != nullptr);
}

TEST_F(AssimpFormatTest, LoadMeshGlb)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Mesh,
		"", "CorruptedWoodFish.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFish.glb";
	EXPECT_TRUE(IO::AssimpFormat::LoadMesh(filePath, metaEntry.loadSettings) != nullptr);
}

TEST_F(AssimpFormatTest, LoadMeshWithMultipleMaterials)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Mesh,
		"", "CorruptedWoodFishMaterial.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFishMaterial.fbx";
	EXPECT_THROW(auto temp = IO::AssimpFormat::LoadMesh(filePath, metaEntry.loadSettings), std::runtime_error);
}

// ==================
// Skinned mesh tests
// ==================

TEST_F(AssimpFormatTest, LoadNonExistingSkinnedMesh)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::SkinnedMesh,
		"", "DoesNotExist.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "EmptyMesh.fbx";
	EXPECT_THROW(auto temp = IO::AssimpFormat::LoadSkinnedMesh(filePath, metaEntry.loadSettings), std::runtime_error);
}

TEST_F(AssimpFormatTest, LoadSkinnedMeshWithNoMeshData)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::SkinnedMesh,
		"", "CorruptedWoodFishWithNoMesh.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFishWithNoMesh.fbx";
	EXPECT_THROW(auto temp = IO::AssimpFormat::LoadSkinnedMesh(filePath, metaEntry.loadSettings), std::runtime_error);
}

TEST_F(AssimpFormatTest, LoadSkinnedMeshWithNoAnimDataButWithSkeleton)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::SkinnedMesh,
		"", "CorruptedWoodFishWithNoAnim.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFishWithNoAnim.fbx";
	EXPECT_TRUE(IO::AssimpFormat::LoadSkinnedMesh(filePath, metaEntry.loadSettings) != nullptr);
}

TEST_F(AssimpFormatTest, LoadSkinnedMeshFbx)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::SkinnedMesh,
		"", "CorruptedWoodFish.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFish.fbx";
	EXPECT_TRUE(IO::AssimpFormat::LoadSkinnedMesh(filePath, metaEntry.loadSettings) != nullptr);
}

TEST_F(AssimpFormatTest, LoadSkinnedMeshGlb)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::SkinnedMesh,
		"", "CorruptedWoodFish.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFish.fbx";
	EXPECT_TRUE(IO::AssimpFormat::LoadSkinnedMesh(filePath, metaEntry.loadSettings) != nullptr);
}

// ===============
// Animation tests
// ===============

TEST_F(AssimpFormatTest, LoadNonExistingAnimation)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Animation,
		"", "DoesNotExist.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "EmptyMesh.fbx";
	EXPECT_THROW(auto temp = IO::AssimpFormat::LoadAnimation(filePath, "", metaEntry.loadSettings), std::runtime_error);
}

TEST_F(AssimpFormatTest, LoadAnimationWithNoMeshData)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Animation,
		"Fish.001|FishAnimation", "CorruptedWoodFishWithNoMesh.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFishWithNoMesh.fbx";
	EXPECT_TRUE(IO::AssimpFormat::LoadAnimation(filePath, metaEntry.sourceName, metaEntry.loadSettings) != nullptr);
}

TEST_F(AssimpFormatTest, LoadAnimationWithNoAnimData)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Animation,
		"Fish.001|FishAnimation", "CorruptedWoodFishWithNoAnim.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFishWithNoAnim.fbx";
	EXPECT_THROW(auto temp = IO::AssimpFormat::LoadAnimation(filePath, metaEntry.sourceName, metaEntry.loadSettings), std::runtime_error);
}

TEST_F(AssimpFormatTest, LoadAnimationFbx)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Animation,
		"Fish.001|FishAnimation", "CorruptedWoodFish.fbx");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFish.fbx";
	EXPECT_TRUE(IO::AssimpFormat::LoadAnimation(filePath, metaEntry.sourceName, metaEntry.loadSettings) != nullptr);
}

TEST_F(AssimpFormatTest, LoadAnimationGlb)
{
	Droplet::MetaEntry metaEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Animation,
		"ArmatureAction", "CorruptedWoodFish.glb");
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFish.glb";
	EXPECT_TRUE(IO::AssimpFormat::LoadAnimation(filePath, metaEntry.sourceName, metaEntry.loadSettings) != nullptr);
}
// ===================
// Asset scanning tests
// ===================

TEST_F(AssimpFormatTest, ScanAssetWithBonesOffersSkinnedMesh)
{
	fs::path filePath = fs::path(TEST_ASSET_DIR) / "Meshes" / "CorruptedWoodFish.fbx";
	auto resources = IO::AssimpFormat::ScanAssetResources(filePath);
	bool hasMesh = false, hasSkinned = false;
	for (const auto &[type, name] : resources)
	{
		hasMesh |= type == Droplet::ResourceType::Mesh;
		hasSkinned |= type == Droplet::ResourceType::SkinnedMesh;
	}
	EXPECT_TRUE(hasMesh);
	EXPECT_TRUE(hasSkinned);
}
