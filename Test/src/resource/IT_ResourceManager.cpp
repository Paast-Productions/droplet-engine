#include "resource/ResourceManager.hpp"
#include "resource/types/Texture2DResource.hpp"
#include "resource/types/ShaderResource.hpp"
#include "core/ThreadPool.hpp"

#include <gtest/gtest.h>
#include <filesystem>
#include <chrono>
#include <thread>

namespace fs = std::filesystem;
using namespace Droplet;

class ResourceManagerTest : public ::testing::Test
{
protected:
    fs::path m_testAssetDir;
    ResourceManager m_resourceManager;

    void SetUp() override
    {
        ThreadPool::GetInstance().Initialize();

        m_testAssetDir = fs::path(TEST_ASSET_DIR);
        m_resourceManager.Initialize(m_testAssetDir);
    }

    void TearDown() override
    {
        ThreadPool::GetInstance().Shutdown();
    }

    // Helper function to simulate the Engine's Main Loop
    // Pumps the manager's Update() function until a resource leaves the Async state, or times out.
    void UpdateUntilResolved(GUID p_guid, int p_timeoutMs = 2000)
    {
        auto startTime = std::chrono::steady_clock::now();
        while (m_resourceManager.GetState(p_guid) == ResourceState::LoadingAsync || 
               m_resourceManager.GetState(p_guid) == ResourceState::Queued)
        {
            m_resourceManager.Update();
            
            if (std::chrono::steady_clock::now() - startTime > std::chrono::milliseconds(p_timeoutMs))
            {
                FAIL() << "Async load timed out for GUID: " << p_guid;
                break;
            }
            std::this_thread::yield();
        }
        
        // One more time just to process any trailing callbacks
        m_resourceManager.Update();
    }
};

TEST_F(ResourceManagerTest, InitializationScansDirectory)
{
    auto textures = m_resourceManager.GetRegisteredResources(ResourceType::Texture2D);
    EXPECT_GE(textures.size(), 1); // Should have found at least Test_BlackPixel.png.meta during setup
}

TEST_F(ResourceManagerTest, LoadInvalidGUID)
{
    GUID invalidGuid = C_INVALID_GUID;
    auto handle = m_resourceManager.LoadResource<Texture2DResource>(invalidGuid);
    EXPECT_FALSE(handle.IsValid());
}

TEST_F(ResourceManagerTest, InvalidGUIDState)
{
    EXPECT_EQ(
        m_resourceManager.GetState(C_INVALID_GUID),
        ResourceState::Unloaded
    );
}

TEST_F(ResourceManagerTest, AsyncLoadResource_LoadCPU_RetainsRAM)
{
    constexpr GUID textureGuid = 500; 
    bool callbackFired = false;

    // Trigger the load (Texture is set to LoadCPU, so RAM should not be cleared)
    ResourceHandle<Texture2DResource> handle = m_resourceManager.LoadResource<Texture2DResource>(
        textureGuid, 
        [&callbackFired](ResourceHandle<Texture2DResource> h) {
            callbackFired = true;
            EXPECT_TRUE(h.IsReady());
            EXPECT_NE(h.Get(), nullptr); // Ensure RAM was kept
        }
    );

    UpdateUntilResolved(textureGuid);

    EXPECT_EQ(m_resourceManager.GetState(textureGuid), ResourceState::Ready);
    EXPECT_TRUE(handle.IsReady());
    EXPECT_NE(handle.Get(), nullptr); // Ensure RAM is still available to the handle
    EXPECT_TRUE(callbackFired);
}

TEST_F(ResourceManagerTest, AsyncLoadResource_LoadGPU_FreesRAM)
{
    constexpr GUID shaderGuid = 600; 
    bool callbackFired = false;

    // Trigger the load (Shader is set to LoadGPU, so RAM should be cleared after upload)
    ResourceHandle<ShaderResource> handle = m_resourceManager.LoadResource<ShaderResource>(
        shaderGuid, 
        [&callbackFired](ResourceHandle<ShaderResource> h) {
            callbackFired = true;
            EXPECT_TRUE(h.IsReady()); // The resource is still considered Ready
            EXPECT_EQ(h.Get(), nullptr); // But the CPU pointer should be NULL
        }
    );

    UpdateUntilResolved(shaderGuid);

    EXPECT_EQ(m_resourceManager.GetState(shaderGuid), ResourceState::Ready);
    EXPECT_TRUE(handle.IsReady());
    EXPECT_EQ(handle.Get(), nullptr); // Ensure RAM was safely freed
    EXPECT_TRUE(callbackFired);
}

TEST_F(ResourceManagerTest, HandleReferenceCounting)
{
    constexpr GUID textureGuid = 500;
    
    {
        // Load the resource
        ResourceHandle<Texture2DResource> handle1 = m_resourceManager.LoadResource<Texture2DResource>(textureGuid);
        EXPECT_EQ(m_resourceManager.GetRef(textureGuid), 1);
        
        {
            // Copy the handle
            ResourceHandle<Texture2DResource> handle2 = handle1;
            EXPECT_EQ(m_resourceManager.GetRef(textureGuid), 2);
            
            // Move the handle
            ResourceHandle<Texture2DResource> handle3 = std::move(handle2);
            EXPECT_EQ(m_resourceManager.GetRef(textureGuid), 2); // Still 2, because handle2 was hollowed out
        }
        
        // handle3 destroyed -> ref count drops back to 1
        EXPECT_EQ(m_resourceManager.GetRef(textureGuid), 1);
    }
    
    // handle1 destroyed -> ref count drops to 0. 
    // Resource should be completely erased from m_liveResources
    EXPECT_EQ(m_resourceManager.GetRef(textureGuid), C_INVALID_GUID);
    EXPECT_EQ(m_resourceManager.GetState(textureGuid), ResourceState::Unloaded);
}

TEST_F(ResourceManagerTest, AsyncAssetRegistration)
{
    std::string targetAsset = "Textures/Test_BlackPixel.png";
    
    // Fire the async registration task
    m_resourceManager.RegisterAsset(targetAsset);
    
    // We can't query state by GUID because it's a registration, so we just update manually
    auto startTime = std::chrono::steady_clock::now();
    
    while (std::chrono::steady_clock::now() - startTime < std::chrono::milliseconds(500))
    {
        m_resourceManager.Update(); // This will eventually process the AsyncRegisterResult
        std::this_thread::yield();
    }
    
    // If CompareAndCompileMetaData worked correctly, it preserved our GUID 500
    // and correctly identified it as a Texture2D.
    auto textures = m_resourceManager.GetRegisteredResources(ResourceType::Texture2D);
    bool found = false;
    
    for (const MetaEntry* entry : textures)
    {
        if (entry->guid == 500 && entry->relAssetPath == "Textures/Test_BlackPixel.png")
        {
            found = true;
            break;
        }
    }
    
    EXPECT_TRUE(found) << "CompareAndCompileMetaData lost or corrupted the metadata during update";
}

TEST_F(ResourceManagerTest, AsyncAssetRegistration_NewAssetGeneratesMeta)
{
    // Copy an existing asset to a new temporary name
    fs::path originalAsset = m_testAssetDir / "Textures/Test_BlackPixel.png"; 
    std::string newAssetRel = "Textures/Test_BrandNewAsset.png";
    
    fs::path newAssetAbs = m_testAssetDir / newAssetRel;
    fs::path newMetaAbs = m_testAssetDir / (newAssetRel + ".meta");

    // Clean up just in case a previous test crashed and left it behind
    if (fs::exists(newAssetAbs)) fs::remove(newAssetAbs);
    if (fs::exists(newMetaAbs)) fs::remove(newMetaAbs);

    // Create the dummy asset so the Loader actually finds valid binary data to parse
    fs::copy(originalAsset, newAssetAbs);

    // Register the new asset
    m_resourceManager.RegisterAsset(newAssetRel);
    
    // Wait
    auto startTime = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - startTime < std::chrono::milliseconds(500))
    {
        m_resourceManager.Update(); 
        std::this_thread::yield();
    }
    
    // Check the internal registry
    auto textures = m_resourceManager.GetRegisteredResources(ResourceType::Texture2D);
    const MetaEntry* foundEntry = nullptr;
    
    for (const MetaEntry* entry : textures)
    {
        if (entry->relAssetPath == newAssetRel)
        {
            foundEntry = entry;
            break;
        }
    }
    
    ASSERT_NE(foundEntry, nullptr) << "Failed to register brand new asset";
    EXPECT_NE(foundEntry->guid, C_INVALID_GUID) << "Failed to generate a valid GUID";
    EXPECT_EQ(foundEntry->type, ResourceType::Texture2D) << "Failed to detect the correct resource type";
    
    // Check the Hard Drive to ensure the metafile was created
    EXPECT_TRUE(fs::exists(newMetaAbs)) << "The new metafile was never written to disk";

    // Clean up
    fs::remove(newAssetAbs);
    fs::remove(newMetaAbs);
}

TEST_F(ResourceManagerTest, LoadSameResourceMultipleTimesQueuesCallbacks)
{
    constexpr GUID textureGuid = 500;
    int callbacksFired = 0;

    auto callback = [&callbacksFired](ResourceHandle<Texture2DResource> h) {
        callbacksFired++;
        EXPECT_TRUE(h.IsReady());
    };

    // Request the same resource 3 times before it finishes loading
    ResourceHandle<Texture2DResource> h1 = m_resourceManager.LoadResource<Texture2DResource>(textureGuid, callback);
    ResourceHandle<Texture2DResource> h2 = m_resourceManager.LoadResource<Texture2DResource>(textureGuid, callback);
    ResourceHandle<Texture2DResource> h3 = m_resourceManager.LoadResource<Texture2DResource>(textureGuid, callback);

    // EXPECT 6: 3 returned to the user + 3 captured by the pending lambdas
    EXPECT_EQ(m_resourceManager.GetState(textureGuid), ResourceState::LoadingAsync);
    EXPECT_EQ(m_resourceManager.GetRef(textureGuid), 6);

    UpdateUntilResolved(textureGuid);

    // All 3 callbacks should have fired from that single load event
    EXPECT_EQ(callbacksFired, 3);
    
    // When callbacks have executed and their internal lambdas were cleared/destroyed the ref count should be 3
    EXPECT_EQ(m_resourceManager.GetRef(textureGuid), 3);
}

TEST_F(ResourceManagerTest, LoadAlreadyLoadedResourceFiresImmediately)
{
    constexpr GUID textureGuid = 500;

    // Fully load the resource first
    ResourceHandle<Texture2DResource> h1 = m_resourceManager.LoadResource<Texture2DResource>(textureGuid);
    UpdateUntilResolved(textureGuid);
    EXPECT_EQ(m_resourceManager.GetState(textureGuid), ResourceState::Ready);

    bool instantCallbackFired = false;

    // Request it again. The callback should fire immediately inline
    ResourceHandle<Texture2DResource> h2 = m_resourceManager.LoadResource<Texture2DResource>(
        textureGuid, 
        [&instantCallbackFired](ResourceHandle<Texture2DResource> h) {
            instantCallbackFired = true;
            EXPECT_TRUE(h.IsReady());
        }
    );
    
    EXPECT_TRUE(instantCallbackFired);
    EXPECT_EQ(m_resourceManager.GetRef(textureGuid), 2);
}

TEST_F(ResourceManagerTest, HandleAssignmentManagesReferences)
{
    constexpr GUID texGuid = 500;
    constexpr GUID shaderGuid = 600;

    ResourceHandle<Texture2DResource> texHandle = m_resourceManager.LoadResource<Texture2DResource>(texGuid);
    ResourceHandle<ShaderResource> shaderHandle = m_resourceManager.LoadResource<ShaderResource>(shaderGuid);

    EXPECT_EQ(m_resourceManager.GetRef(texGuid), 1);
    EXPECT_EQ(m_resourceManager.GetRef(shaderGuid), 1);

    // Reassign the texture handle to a new texture handle
    ResourceHandle<Texture2DResource> texHandle2;
    texHandle2 = texHandle; // Ref count should go to 2

    EXPECT_EQ(m_resourceManager.GetRef(texGuid), 2);

    // Overwrite texHandle2 with an empty handle
    texHandle2 = ResourceHandle<Texture2DResource>(); // Ref count drops back to 1
    
    EXPECT_EQ(m_resourceManager.GetRef(texGuid), 1);
}

TEST_F(ResourceManagerTest, AsyncLoadFailureTriggersCallbackAndState)
{
    constexpr GUID missingGuid = 700; // A GUID registered to a file that doesn't exist

    bool callbackFired = false;

    ResourceHandle<Texture2DResource> handle = m_resourceManager.LoadResource<Texture2DResource>(
        missingGuid, 
        [&callbackFired](ResourceHandle<Texture2DResource> h) {
            callbackFired = true;
            
            // The handle should exist, but be marked as failed
            EXPECT_FALSE(h.IsReady());
            EXPECT_TRUE(h.HasFailed());
            EXPECT_EQ(h.Get(), nullptr);
        }
    );

    UpdateUntilResolved(missingGuid);

    EXPECT_EQ(m_resourceManager.GetState(missingGuid), ResourceState::Failed);
    EXPECT_TRUE(callbackFired);
}

