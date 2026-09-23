#include <print>

#include "asset/ResourceManager.hpp"
#include "asset/AssimpLoader.hpp"
#include "asset/TextureLoader.hpp"

#include <filesystem>
#include <string>
#include <iostream>

namespace fs = std::filesystem;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    std::print("Starting: Asset Loading Sample\n");
    
    fs::path assetDir = fs::current_path() / fs::path("assets");
    std::println("Initializing AssetManager for dir: {}", assetDir.generic_string());
    
    Droplet::ResourceManager assetManager;
    assetManager.Initialize(assetDir);

    Droplet::AssimpLoader assimpLoader;
    Droplet::AssetRecord assetRecord;
    assimpLoader.LoadAnimation("CorruptedWoodFish.fbx", "Fish|ArmatureAction", nlohmann::json(), assetRecord);
    
    Droplet::ResourceLoader::TextureLoader resourceTexture;

    fs::path currentPath = fs::current_path();
    std::string path = "";
    std::cout << "Does file path exist? " << std::filesystem::exists(path) << std::endl;

    try
    {
        resourceTexture.Load(path);
    }
    catch (std::exception) {
        std::println("Failed to load texture.");
    }


    return 0;
}