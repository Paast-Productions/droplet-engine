#include <print>

#include "asset/AssetManager.hpp"

#include <filesystem>

namespace fs = std::filesystem;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    std::print("Starting: Asset Loading Sample\n");
    
    fs::path assetDir = fs::current_path() / fs::path("assets");
    std::println("Initializing AssetManager for dir: {}", assetDir.generic_string());
    
    engine::AssetManager assetManager;
    assetManager.Initialize(assetDir);
    
    return 0;
}