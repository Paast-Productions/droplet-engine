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
	class ResourceTexture /* : public IResource */
	{
	public:

		/// @return True if DDS or KTX is succesully loaded 
		bool Load(const std::string &path);

		/// @return True if PNG succesfully converted to KTX
		bool ConvertPNGToKTX(const std::string& inputPath, const std::string& outputPath);

		/// @return the size of the texture
		size_t GetSize() const
		{
			return m_texture.size();
		}

		/// @return the width of the texture
		uint32_t GetWidth() const
		{
			return static_cast<uint32_t>(m_texture.extent().x);
		}

		/// @return the height of the texture
		uint32_t GetHeight() const
		{
			return static_cast<uint32_t>(m_texture.extent().y);
		}
		/// @return the amount of mip levels of the texture
		uint32_t GetMipLevels() const
		{
			return static_cast<uint32_t>(m_texture.levels());
		}

		/// @return the gli format of the texture
		gli::format GetFormat() const
		{
			return m_texture.format();
		}
	private: 
		gli::texture m_texture;
	};
}