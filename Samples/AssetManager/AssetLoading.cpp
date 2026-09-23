#include <print>

#include "asset/AssetManager.hpp"
#include "asset/AssimpLoader.hpp"

#include <filesystem>

namespace fs = std::filesystem;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    std::print("Starting: Asset Loading Sample\n");
    
    fs::path assetDir = fs::current_path() / fs::path("assets");
    std::println("Initializing AssetManager for dir: {}", assetDir.generic_string());
    
    Droplet::AssetManager assetManager;
    assetManager.Initialize(assetDir);

    Droplet::AssimpLoader assimpLoader;
    Droplet::AssetRecord assetRecord;
    assimpLoader.LoadAnimation("CorruptedWoodFish.fbx", "Fish|ArmatureAction", nlohmann::json(), assetRecord);
    
    return 0;
}