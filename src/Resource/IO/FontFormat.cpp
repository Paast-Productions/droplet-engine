#include "resource/io/FontFormat.hpp"

#include "resource/IStaticResource.hpp"
#include "resource/meta/MetaUtils.hpp"

#include <glm/gtc/type_precision.hpp>

using namespace Droplet;
using namespace Droplet::IO::FontFormat;

std::unique_ptr<FontResource> IO::FontFormat::LoadFont(const std::filesystem::path &p_fontPath)
{
	ZoneScoped
	
	if (p_fontPath.extension() != ".slug")
	{
		throw std::runtime_error("Font file must be of type .slug");
	}
	
	std::ifstream inFile(p_fontPath.string(), std::ios::binary);
	if (!inFile.is_open())
	{
		throw std::runtime_error(std::format("Could not open file at {}", p_fontPath.string()));
	}
	
	char header[FontResource::SLUG_HEADER_LEN + 1];
	inFile.read(header, FontResource::SLUG_HEADER_LEN);
	header[FontResource::SLUG_HEADER_LEN] = '\0';
	if (strcmp(header, FontResource::SLUG_HEADER_DATA) != 0)
	{
		throw std::runtime_error(std::format("Invalid header found ({} instead of {})", header, FontResource::SLUG_HEADER_DATA));
	}
	
	char version[FontResource::SLUG_FILE_VERSION_LEN + 1];
	inFile.read(version, FontResource::SLUG_FILE_VERSION_LEN);
	version[FontResource::SLUG_FILE_VERSION_LEN] = '\0';
	if (strcmp(version, FontResource::SLUG_FILE_VERSION) != 0)
	{
		throw std::runtime_error(std::format("Slug file version mismatch! Expected version number {} but got {}. ", FontResource::SLUG_FILE_VERSION, version));
	}
	
	FontResource::LoadedFontData data{};
	data.name = p_fontPath.stem().string();
	
	inFile.read(reinterpret_cast<char *>(&data.fontHeader), sizeof(FontResource::SlugFontHeader));
	
	uint16_t codePointCount = 0;
	inFile.read(reinterpret_cast<char *>(&codePointCount), sizeof(codePointCount));
	if (codePointCount == 0)
	{
		throw std::runtime_error("No code points found.");
	}
	
	data.glyphs.resize(codePointCount);
	inFile.read(reinterpret_cast<char *>(data.glyphs.data()), static_cast<std::streamsize>(data.glyphs.size() * sizeof(FontResource::SlugGlyph)));
	
	uint32_t curveTextureBytes;
	inFile.read(reinterpret_cast<char *>(&data.curveTextureWidth), sizeof(data.curveTextureWidth));
	inFile.read(reinterpret_cast<char *>(&data.curveTextureHeight), sizeof(data.curveTextureHeight));
	inFile.read(reinterpret_cast<char *>(&curveTextureBytes), sizeof(curveTextureBytes));
	if (data.curveTextureWidth == 0 || data.curveTextureHeight == 0 || curveTextureBytes == 0 || data.curveTextureWidth != FontResource::TEXTURE_WIDTH)
	{
		throw std::runtime_error("Invalid curve texture dimensions.");
	}
	
	// Number of texels in the curve texture
	const size_t curveTexTexels = static_cast<size_t>(data.curveTextureWidth * data.curveTextureHeight);
	data.curvesTextureData.resize(curveTexTexels);
	
	size_t capacityBytes = data.curvesTextureData.size() * sizeof(glm::vec4);
	if (curveTextureBytes > capacityBytes)
	{
		throw std::runtime_error("Curve payload exceeds allocated capacity.");
	}
	
	memset(data.curvesTextureData.data(), 0, data.curvesTextureData.size() * sizeof(data.curvesTextureData.front()));
	if (!inFile.read(reinterpret_cast<char *>(data.curvesTextureData.data()), curveTextureBytes))
	{
		throw std::runtime_error("Failed to read curve texture payload.");
	}
	
	uint32_t bandsTextureBytes;
	inFile.read(reinterpret_cast<char *>(&data.bandsTextureWidth), sizeof(data.bandsTextureWidth));
	inFile.read(reinterpret_cast<char *>(&data.bandsTextureHeight), sizeof(data.bandsTextureHeight));
	inFile.read(reinterpret_cast<char *>(&bandsTextureBytes), sizeof(bandsTextureBytes));
	if (data.bandsTextureWidth == 0 || data.bandsTextureHeight == 0 || bandsTextureBytes == 0 || data.bandsTextureWidth != FontResource::TEXTURE_WIDTH)
	{
		throw std::runtime_error("Invalid bands texture dimensions.");
	}
	
	
	// Number of texels in the bands texture
	const size_t bandsTexTexels = static_cast<size_t>(data.bandsTextureWidth * data.bandsTextureHeight);
	data.bandsTextureData.resize(bandsTexTexels);
	
	capacityBytes = data.bandsTextureData.size() * sizeof(glm::u16vec2);
	if (bandsTextureBytes > capacityBytes)
	{
		throw std::runtime_error("Bands payload exceeds allocated capacity.");
	}
	
	memset(data.bandsTextureData.data(), 0, data.bandsTextureData.size() * sizeof(data.bandsTextureData.front()));
	if (!inFile.read(reinterpret_cast<char *>(data.bandsTextureData.data()), bandsTextureBytes))
	{
		throw std::runtime_error("Failed to read curve texture payload.");
	}
	
	return std::make_unique<FontResource>(std::move(data));
}

std::vector<std::pair<StaticResourceType, std::string>> IO::FontFormat::ScanAssetResources(
	const std::filesystem::path &p_assetPath)
{
	ZoneScoped
	
	std::vector<std::pair<StaticResourceType, std::string>> resources{};
	
	std::string resourceName = p_assetPath.stem().string();
	resources.emplace_back(StaticResourceType::Font, resourceName);
	return resources;
}

