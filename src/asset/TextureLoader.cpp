#include "Asset/TextureLoader.hpp"
#include <iostream>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include <Stb/stb_image.h>

namespace fs = std::filesystem;
namespace Droplet
{
	gli::texture TextureLoader::Load(const std::string &path)
	{
		gli::texture texture;
		fs::path filePath(path);

		if (!fs::exists(filePath))
		{
			std::cout << "File does not  exist: " << path << std::endl;
			return {};
		}

		std::string extension = filePath.extension().string();

		if (extension == ".ktx" || extension == ".dds")
		{
			texture = gli::load(filePath.string());
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
			std::cout << "Could not load texture" << std::endl
				<< path << std::endl;
	
			return {};
		}
		return texture;
	}

	bool TextureLoader::ConvertPNGToKTX(const std::string& inputPath, const std::string& outputPath)
	{
		int width = 0;
		int height = 0;
		int channels = 0;

		stbi_uc* data = stbi_load(inputPath.c_str(), &width, &height, &channels, 4);

		if (!data)
		{
			std::cout << "Could not load PNG file" << std::endl;
			return false;
		}

		gli::texture texture;

		std::memcpy(texture.data(), data, width * height * 4);

		stbi_image_free(data);

		if (!gli::save(texture, outputPath))
		{
			std::cout << "Could not save KTX file" << std::endl;
			return {};
		}

		texture = gli::load(outputPath);
		return true;
	}
}