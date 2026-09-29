#include "resource/loaders/TextureLoader.hpp"
#include <iostream>
#include <filesystem>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <Stb/stb_image.h>
#undef STB_IMAGE_IMPLEMENTATION

namespace fs = std::filesystem;
namespace Droplet
{
	std::unique_ptr<Texture2DResource> TextureLoader::Load(const std::string &p_path)
	{
		gli::texture texture;
		fs::path filePath(p_path);

		if (!fs::exists(filePath))
		{
			std::cout << "File not found: " << filePath << std::endl;
			throw std::runtime_error("File was not found");
		}

		std::string extension = filePath.extension().string();
		if (extension == ".ktx" || extension == ".dds")
		{
			if (fs::file_size(filePath) == 0)
			{
				throw std::runtime_error("Texture fil is empty: " + filePath.string());
			}
			texture = gli::load(filePath.string());
		}
		else if(extension == ".png")
		{
			fs::path inputPath = filePath;
			filePath.replace_extension(".ktx");
			ConvertPNGToKTX(inputPath.string(), filePath.string());
			
			texture = gli::load(filePath.string());
		}
		else
		{
			throw std::runtime_error("File format is invalid");
		}

		if (texture.empty())
		{
			throw std::runtime_error("Loaded Texture is empty: " + p_path);
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

	bool TextureLoader::ConvertPNGToKTX(const std::string &p_inputPath, const std::string &p_outputPath)
	{
		int width = 0;
		int height = 0;
		int channels = 0;
		stbi_uc* data = stbi_load(p_inputPath.c_str(), &width, &height, &channels, 4);

		if (!data)
		{
			std::cout << "Could not load PNG file" << std::endl;
			return false;
		}

		gli::texture2d texture(
			gli::FORMAT_RGBA8_UNORM_PACK8,
			gli::extent2d(width, height),
			1
		);
		
		std::memcpy(texture.data(), data, width * height * 4);
		if (texture.empty())
		{
			std::cout << "Print when texture is empty" << std::endl;
			return {};
		}

		stbi_image_free(data);

		return gli::save(texture, p_outputPath);
	}

	TextureResource::TextureFormat TextureLoader::ConvertTextureFormat(gli::format p_format)
	{
		switch (p_format)
		{
		case gli::FORMAT_R8_UNORM_PACK8:
			return TextureResource::TextureFormat::R8_Unorm;

		case gli::FORMAT_RGB8_UNORM_PACK8:
			return TextureResource::TextureFormat::RGB8_Unorm;

		case gli::FORMAT_RGBA8_UNORM_PACK8:
			return TextureResource::TextureFormat::RGBA8_Unorm;

		case gli::FORMAT_RGBA_BP_UNORM_BLOCK16:
			return TextureResource::TextureFormat::BC7;
		default:
			return TextureResource::TextureFormat::Unknown;
		}
	}
}