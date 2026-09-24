#pragma once

// include function from gli library specifically

#include <gli/load.hpp>
#include <gli/texture.hpp>
#include <gli/texture2d.hpp>
#include <gli/save.hpp>

#include "resource/types/Texture2DResource.hpp"

#include <string>

namespace Droplet
{
	/// @brief Texture resource that holds a gli texture. Can load dds and
	/// KTX files but also convert PNG files to KTX format 
	class TextureLoader
	{
	public:
		/// @brief Loads a texture from the specified path.
		/// @return A unique pointer to the texture file.
		/// @param p_path String of textures path in explorer.
		/// @throws std::runtime_error If the texture cannot be found or loaded.
		std::unique_ptr<Texture2DResource> Load(const std::string &p_path);
	
	private:
		/// @brief Converts a PNG texture to KTX and stores the asset file
		/// @return True if PNG succesfully converted to KTX.
		/// @param p_inputPath The path to the asset file that stores the texture.
		/// @param p_outputPath The new path to the asset file that stores the texture..
		/// @throws std::runtime_error If the texture cannot be found or loaded.
		bool ConvertPNGToKTX(const std::string &p_inputPath, const std::string &p_outputPath);

		/// @brief Converts a gli::format to our custom ResourceTexture::TextureFormat
		/// @return The equvialent format of gli::texture as TextureFormat
		/// @param 
		Texture2DResource::TextureFormat ConvertTextureFormat(gli::format p_format);

	};
}