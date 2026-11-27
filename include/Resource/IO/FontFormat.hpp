#pragma once

#include "resource/types/FontResource.hpp"

namespace Droplet::IO::FontFormat
{
	[[nodiscard]] std::unique_ptr<FontResource> LoadFont(const std::filesystem::path &p_fontPath);
	
	[[nodiscard]] std::vector<std::pair<StaticResourceType, std::string>> ScanAssetResources(const std::filesystem::path &p_assetPath);
}