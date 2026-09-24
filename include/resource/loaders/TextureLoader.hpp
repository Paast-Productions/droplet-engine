#pragma once

// include function from gli library specifically

#include <gli/load.hpp>
#include <gli/texture.hpp>
#include <gli/texture2d.hpp>
#include <gli/save.hpp>

#include "asset/Resource.hpp"

#include <string>

namespace Droplet::ResourceLoader
{
	/// @brief Texture resource that holds a gli texture. Can load dds and
	/// KTX files but also convert PNG files to KTX format 
	class TextureLoader
	{
	public:

		/// @return Texture2DResource
		/// @param p_path string of textures path in explorer
		/// @throws the invalid path
		std::unique_ptr<Texture2DResource> Load(const std::string &p_path);

		/// @return True if PNG succesfully converted to KTX
		/// @param p_inputPath string of textures path in explorer
		/// @param p_outputPath string of new path to texture in explorer
		/// @throws the invalid path
		bool ConvertPNGToKTX(const std::string &p_inputPath, const std::string &p_outputPath);

		Texture2DResource::TextureFormat ConvertTextureFormat(gli::format p_format);

	};
}