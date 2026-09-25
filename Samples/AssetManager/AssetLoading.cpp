#include "resource/ResourceManager.hpp"
#include "resource/loaders/GliLoader.hpp"

#include "resource/loaders/AssimpLoader.hpp"
#
#include "resource/ThreadPool.hpp"
#include "resource/meta/MetaData.hpp"
#include "resource/meta/MetaUtils.hpp"
#include "resource/types/Texture2DResource.hpp"

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
    
    Droplet::ResourceRecord assetRecord;
    //assetRecord.resource = assimpLoader.LoadAnimation("CorruptedWoodFish.fbx", "Fish|ArmatureAction", nlohmann::json());
    
    fs::path currentPath = fs::current_path();
    std::string path = "C:/Users/Proj/Documents/Blender Assets/Test_BlackPixel.ktx";
    std::vector<Droplet::MetaEntry> metaData;

    std::filesystem::path metaPath = path + ".meta";
    // If meta file does not exist, generate a new so that you can use GUID
    /*auto newEntry = Droplet::MetaUtils::GenerateDefaultMetaEntry(Droplet::ResourceType::Texture2D, "Test_BlackPixel.ktx", path);
    metaData.push_back(newEntry);
    Droplet::MetaUtils::Write(metaPath, metaData);*/

    
    // If meta file does exist, Read the path and extract the data
    Droplet::MetaUtils::Read(metaPath, metaData);

    //


    try
    {
        // Ska ha GUID
        for (auto &entry : metaData)
        {
            // Since metaData is a vector we need to iterate all entries to get GUID
            auto handle = assetManager.LoadResource<Droplet::Texture2DResource>(entry.guid);
            //auto handle1 = assetManager.LoadResource<Droplet::Texture2DResource>(entry.guid);
            auto handle1 = handle;
            std::cout << "handle1 RefCount: " << handle1.GetRef() <<std::endl;
            std::cout << "handle RefCount: " << handle.GetRef() <<std::endl;
            
        }
    }
    catch (const std::exception &e) {
        std::cout << "Could not load texture from main Error message: " << e.what() << std::endl;;
    }

    return 0;
}