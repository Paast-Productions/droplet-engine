#include <print>

#include "asset/AssetManager.hpp"

#include "asset/ResourceTexture.hpp"

#include <filesystem>
#include <string>
#include <iostream>

namespace fs = std::filesystem;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    std::print("Starting: Asset Loading Sample\n");
    
    fs::path assetDir = fs::current_path() / fs::path("assets");
    std::println("Initializing AssetManager for dir: {}", assetDir.generic_string());
    
    Droplet::AssetManager assetManager;
    assetManager.Initialize(assetDir);
    
    Droplet::ResourceTexture resourceTexture;

    fs::path currentPath = fs::current_path();
    std::string path = "basic_fish_colors_base.png";
    std::cout << "Does file path exist? " << std::filesystem::exists(path) << std::endl;

    resourceTexture.Load(path);


    return 0;
}