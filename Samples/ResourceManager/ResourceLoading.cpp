#include "resource/ResourceManager.hpp"
#include "resource/loaders/GliLoader.hpp"

#include "resource/loaders/AssimpLoader.hpp"
#
#include "core/ThreadPool.hpp"
#include "resource/meta/MetaData.hpp"
#include "resource/meta/MetaUtils.hpp"
#include "resource/types/Texture2DResource.hpp"

#include <filesystem>
#include <string>
#include <iostream>
#include <print>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace fs = std::filesystem;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    std::println("Starting: Asset Loading Sample");
    
    // Simulate engine initialize
    Droplet::ThreadPool::GetInstance().Initialize();
    
    // Initialize resource manager
    fs::path assetDir = fs::current_path() / fs::path("assets");
    std::println("Initializing AssetManager for dir: {}", assetDir.generic_string());
    Droplet::ResourceManager resourceManager;
    resourceManager.Initialize(assetDir);
    
    // Register resource types
    resourceManager.RegisterResourceType<Droplet::Texture2DResource>();
    resourceManager.RegisterResourceType<Droplet::MeshResource>();
    resourceManager.RegisterResourceType<Droplet::SkinnedMeshResource>();
    resourceManager.RegisterResourceType<Droplet::AnimationResource>();
    resourceManager.RegisterResourceType<Droplet::ShaderResource>();
    
    // Register an asset
    resourceManager.RegisterAsset(assetDir / "CorruptedWoodFish.fbx");
    Sleep(5);
    resourceManager.Update();
    
    // Simulate engine shutdown
    Droplet::ThreadPool::GetInstance().Shutdown();
    return 0;
}