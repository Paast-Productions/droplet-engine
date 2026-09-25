#include "resource/loaders/GliLoader.hpp"

#include "MetaUtils.hpp"

#include <iostream>
#include <filesystem>
#include <stdexcept>

#include <gli/load.hpp>
#include <gli/texture.hpp>
#include <gli/texture2d.hpp>
#include <gli/save.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <Stb/stb_image.h>
#undef STB_IMAGE_IMPLEMENTATION

namespace fs = std::filesystem;

namespace Droplet::GliLoader
{
    /// @brief Converts a gli::format to our custom ResourceTexture::TextureFormat.
    /// @param p_format The GLI format.
    /// @return The equivalent format of gli::texture as TextureFormat.
    [[nodiscard]] static TextureResource::TextureFormat ConvertTextureFormat(gli::format p_format)
    {
        switch (p_format)
        {
        case gli::FORMAT_R8_UNORM_PACK8:
            std::cout << "TextureResourceFormat: R8_UNORM" << std::endl;
            return TextureResource::TextureFormat::R8_Unorm;

        case gli::FORMAT_RGB8_UNORM_PACK8:
            std::cout << "TextureResourceFormat: RGB8_UNORM" << std::endl;
            return TextureResource::TextureFormat::RGB8_Unorm;

        case gli::FORMAT_RGBA8_UNORM_PACK8:
            std::cout << "TextureResourceFormat: RGBA8_UNORM" << std::endl;
            return TextureResource::TextureFormat::RGBA8_Unorm;

        case gli::FORMAT_RGBA_BP_UNORM_BLOCK16:
            std::cout << "TextureResourceFormat: BC7" << std::endl;
            return TextureResource::TextureFormat::BC7;
        default:
            std::cout << "TextureResourceFormat: Unknown" << std::endl;
            return TextureResource::TextureFormat::Unknown;
        }
    }
    
    /// @brief Converts a PNG texture to KTX and stores the asset file
    /// @return True if PNG successfully converted to KTX.
    /// @param p_inputPath The path to the asset file that stores the texture.
    /// @param p_outputPath The new path to the asset file that stores the texture.
    /// @throws std::runtime_error If the texture cannot be found or loaded.
    [[nodiscard]] static gli::texture ConvertPNGToKTX(const fs::path &p_inputPath, const fs::path &p_outputPath)
    {
        int width = 0;
        int height = 0;
        int channels = 0;
        
        std::string inputPathStr = p_inputPath.generic_string();
        stbi_uc* data = stbi_load(inputPathStr.c_str(), &width, &height, &channels, 4);

        if (!data)
        {
            throw std::runtime_error("Could not load PNG file: " + inputPathStr);
        }
        
        // Instantiate a 2D texture with allocated memory
        gli::texture2d texture2D(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(width, height), 1);
        std::memcpy(texture2D.data(), data, width * height * 4);
        stbi_image_free(data);

        if (!gli::save(texture2D, p_outputPath.generic_string()))
        {
            // TODO: Use logger to log error
            std::cout << "Could not save KTX file" << std::endl;
        }
        
        return texture2D;
    }
    
	std::unique_ptr<Texture2DResource> LoadTexture2D(const fs::path &p_assetPath, const nlohmann::json &p_loadSettings)
	{
        bool generateMipMaps = p_loadSettings.value("generate_mipmaps", MetaUtils::C_TEXTURE_DEFAULT_GENERATE_MIPMAPS);
        // TODO: Use load setting
        // From what I can gather, we can either bake mipmaps into the ktx file, generate them at load time (probably bad)
        // Or generate them when uploading to GPU. Either way we need to use the load setting in some way here.
        
		gli::texture texture;

		if (!fs::exists(p_assetPath))
		{
			throw std::runtime_error("File was not found");
		}

		std::string extension = p_assetPath.extension().string();

		if (extension == ".ktx" || extension == ".dds")
		{
			texture = gli::load(p_assetPath.string());
			std::cout << texture.format() << std::endl;
		}
		else
		{
			fs::path outputPath = p_assetPath;
			outputPath.replace_extension(".ktx");
			std::cout << outputPath.string() << std::endl;
			texture = ConvertPNGToKTX(p_assetPath, outputPath);
		    // NOTE: Doing this during runtime could be problematic as two threads could potentially try to read/write
		    // to the same file at the same time. We should change this!
		}

		if (texture.empty())
		{
			throw std::runtime_error("Could not load texture: " + p_assetPath.generic_string());
		}

		// Translate the gli texture to our custom format
		TextureResource::TextureFormat format;
		format = ConvertTextureFormat(texture.format());

		auto resource = std::make_unique<Texture2DResource>();
		resource->SetDimensions(texture.extent().x, texture.extent().y);
		resource->SetMipLevels(static_cast<int>(texture.levels()));
		resource->SetPixelData(texture.data(), texture.size());
		resource->SetFormat(format, static_cast<int>(texture.size()));

		return resource;
	}
    
    std::vector<std::pair<ResourceType, std::string>> ListAssetResources(
    const std::filesystem::path &p_assetPath)
    {
        std::vector<std::pair<ResourceType, std::string>> resources{};
        
        std::string resourceName = p_assetPath.stem().string();
        resources.emplace_back(ResourceType::Texture2D, resourceName);
        return resources;
    }
}