#pragma once

//#include <gli/gli.hpp>
//#include <glm/glm.hpp>

#pragma warning(push)
#pragma warning(disable : 4458)
//#define GLM_ENABLE_EXPERIMENTAL

// include function from gli library specifically
#include <gli/load.hpp>
#include <gli/texture.hpp>
#include <gli/texture2d.hpp>
#include <gli/save.hpp>

#include <gli/save.hpp>

#include <string>

#pragma warning(pop)
namespace Droplet
{
	class ResourceTexture /* : public IResource */
	{
	public:
		bool Load(const std::string &path);
		bool ConvertPNGToKTX(const std::string& inputPath, const std::string& outputPath);
	private: 
		gli::texture m_texture;
	};
}