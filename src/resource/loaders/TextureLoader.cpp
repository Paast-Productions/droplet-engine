#include "resource/loaders/TextureLoader.hpp"
#include <iostream>
#include <filesystem>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <Stb/stb_image.h>
#undef STB_IMAGE_IMPLEMENTATION

namespace fs = std::filesystem;
namespace Droplet::ResourceLoader
{
	gli::texture TextureLoader::Load(const std::string &p_path)
	{
		gli::texture texture;
		fs::path filePath(p_path);

		if (!fs::exists(filePath))
		{
			throw std::runtime_error("File was not found");
		}

		std::string extension = filePath.extension().string();

		if (extension == ".ktx" || extension == ".dds")
		{
			texture = gli::load(filePath.string());
			std::cout << texture.format() << std::endl;
		}
		else
		{
			fs::path inputPath = filePath;
			filePath.replace_extension(".ktx");
			std::cout << inputPath.string() << std::endl;
			ConvertPNGToKTX(inputPath.string(), filePath.string());
		}

		if (texture.empty())
		{
			throw std::runtime_error("Could not load texture: " + p_path);
		}
		return texture;
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

		gli::texture texture;

		std::memcpy(texture.data(), data, width * height * 4);

		stbi_image_free(data);

		if (!gli::save(texture, p_outputPath))
		{
			std::cout << "Could not save KTX file" << std::endl;
			return {};
		}

		texture = gli::load(p_outputPath);
		return true;
	}
}