#include "resource/ResourceManager.hpp"
#include "resource/loaders/TextureLoader.hpp"

#include "resource/ResourceManager.hpp"
#include "resource/AssimpLoader.hpp"
#include "resource/loaders/TextureLoader.hpp"

#include "asset/ThreadPool.hpp"
//#include <thread>
#include "asset/meta/MetaData.hpp"
#include "asset/meta/MetaUtils.hpp"

#include <filesystem>
#include <string>
#include <iostream>
#include <print>

namespace fs = std::filesystem;

void PrintSomeThing()
{
    std::cout << "Hi from Thread"
        << std::this_thread::get_id() 
        << std::endl;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    std::print("Starting: Asset Loading Sample\n");
    
    fs::path assetDir = fs::current_path() / fs::path("assets");
    std::println("Initializing AssetManager for dir: {}", assetDir.generic_string());
    
    Droplet::ResourceManager assetManager;
    assetManager.Initialize(assetDir);

    Droplet::AssimpLoader assimpLoader;
    Droplet::ResourceRecord assetRecord;
    assetRecord.resource = assimpLoader.LoadAnimation("CorruptedWoodFish.fbx", "Fish|ArmatureAction", nlohmann::json());
    
    Droplet::ResourceLoader::TextureLoader resourceTexture;

   /* Droplet::ResourceLoader::TextureLoader resourceTexture;

    fs::path currentPath = fs::current_path();
    std::string path = "";
    std::vector<Droplet::MetaEntry> metaData;
     
    //auto entry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Texture2D, "basic_fish_colors_base", path);

    //metaData.push_back(entry);

    std::filesystem::path metaPath = path + ".meta";

    Droplet::MetaUtils::Read(metaPath, metaData);

    

    try
    {
        resourceTexture.Load(path);
    }
    catch (std::exception) {
        std::println("Failed to load texture.");
    }


    return 0;
}