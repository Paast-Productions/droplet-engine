#pragma once

// include function from gli library specifically
#include <gli/load.hpp>
#include <gli/texture.hpp>
#include <gli/texture2d.hpp>
#include <gli/save.hpp>

#include <gli/save.hpp>

#include <string>

namespace Droplet
{
	/// @brief Texture resource that holds a gli texture. Can load dds and
	/// KTX files but also convert PNG files to KTX format 
	class TextureLoader /* : public IResource */
	{
	public:

		/// @return loaded gli texture 
		gli::texture Load(const std::string &path);

		/// @return True if PNG succesfully converted to KTX
		bool ConvertPNGToKTX(const std::string& inputPath, const std::string& outputPath);

	};
}