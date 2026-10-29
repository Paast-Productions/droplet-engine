#include "FontProcessor.hpp"
#include "Logger.hpp"
#include "File.hpp"
#include "GlyphTransaction.hpp"

#include <Stb/stb_truetype.h>

#include "foo.hpp"

Droplet::Devtools::SlugGenerator::FontProcessor::FontProcessor(const std::filesystem::path &p_path, const uint32_t p_bandCount, const int p_asciiRangeStart, const int p_asciiRangeEnd)
	: m_path(p_path), m_bandCount(p_bandCount)
{
	assert(p_bandCount > 0); // We must have a number of bands.
	
	// TODO: Check over the failure cases. When should we fatally fail etc?
	m_isValid = false;
	Logger::PrintInfo("Processing {}...", m_path.filename().string());
	
	if (!OpenFont())
		return;
	
	// The range of ascii characters to process (inclusive)
	// TODO: Support for extended ascii codes, such as the Swedish alphabet. This will require some change in how we loop below.
	int successfullyProcessed = 0;
	std::string foo = "";
	for (int codePoint = p_asciiRangeStart; codePoint <= p_asciiRangeEnd; ++codePoint)
	{
		if (!ProcessCodePoint(codePoint))
		{
			foo += (char)codePoint;
			continue;
		}
		
		++successfullyProcessed;
	}
	
	Logger::PrintInfo("Finished processing {}.", m_path.filename().string());
	
	int totalCount = p_asciiRangeEnd - p_asciiRangeStart + 1;
	Logger::PrintInfo("{} / {} code points successfully processed. ", successfullyProcessed, totalCount);
	
	if (successfullyProcessed != totalCount)
		Logger::PrintWarning("Missing characters - {}", foo);
	
	if (SaveData())
	{
		Logger::PrintInfo("Successfully saved font data to {}.", "fooPath");
	}
	else
	{
		Logger::PrintError("Could not save font data.");
	}
	
	Logger::PrintEmpty();
	m_isValid = true;
}

bool Droplet::Devtools::SlugGenerator::FontProcessor::OpenFont()
{
	// Open the source font file
	File file(m_path);
	if (!file.IsOpen())
		return Error("Could not open font file.");
		
	// This must be kept alive until m_fontInfo is no longer used.
	if (!file.ReadAll(m_fontData))
		return Error("Could not read font data."); // The file is closed when the File class is destructed.
	
	file.Close(); // The file is no longer needed.
		
	// Get the font info
	if (!stbtt_InitFont(&m_fontInfo, reinterpret_cast<const unsigned char *>(m_fontData.data()), 0))
		return Error("Could not get font information.");
	
	return true;
}

bool Droplet::Devtools::SlugGenerator::FontProcessor::ProcessCodePoint(const int p_codePoint)
{
	const int glyphIdx = stbtt_FindGlyphIndex(&m_fontInfo, p_codePoint);
	if (glyphIdx == 0)
	{
		Logger::PrintWarning("Font does not contain code point U+{:04X}.", static_cast<uint32_t>(p_codePoint));
		return false;
	}
	
	// By using a unique_ptr with a custom deleter, the allocated memory for rawVertices is automatically freed.
	stbtt_vertex* rawVertices = nullptr;
	const int vertexCount = stbtt_GetGlyphShape(&m_fontInfo, glyphIdx, &rawVertices);
	std::unique_ptr<stbtt_vertex, StbVertexDeleter> vertices (rawVertices, StbVertexDeleter{&m_fontInfo});
	
	if (vertexCount <= 0)
	{
		Logger::PrintError("No vertices found for code point {}.", p_codePoint);
		return false;
	}
	
	for (int v = 0; v < vertexCount; ++v)
	{
		// TODO: Add bicubic curve support
		if (vertices.get()[v].type == STBTT_vcubic)
		{
			Logger::PrintError("Code point {} uses bicubic curves, which are not supported currently.", p_codePoint);
			return false;
		}
	}
	
	GlyphBounds bounds = GetGlyphBounds(glyphIdx);
	
	std::vector<Curve> curves;
	curves.reserve(static_cast<size_t>(vertexCount));
	
	// Local lambda function to calculate glyph offset
	const glm::vec2 glyphOffset = { bounds.GetMinX(), bounds.GetMinY() };
	
	{
		Curve curve{};
		for (int v = 0; v < vertexCount; ++v)
		{
			const stbtt_vertex& vertex = vertices.get()[v];
			if (vertex.type == STBTT_vmove) // This vertex defines the starting point of a contour
			{
				curve.first = true;
				curve.p3 = ToGlyphLocal(glyphOffset, vertex.x, vertex.y);
				continue;
			}
		
			// Should not occur, but early out to be safe
			if (vertex.type != STBTT_vcurve && vertex.type != STBTT_vline)
			{
				Logger::PrintWarning("Encountered unexpected vertex type on U+{:04X}.", p_codePoint);
				continue;
			}
	
			// The end of the previous curve makes out the start of the new one
			curve.p1 = curve.p3;
		
			// Translate the end point
			curve.p3 = ToGlyphLocal(glyphOffset, vertex.x, vertex.y);
		
			if (vertex.type == STBTT_vcurve) // For a curve, translate the second point to the glyph bounds
				curve.p2 = ToGlyphLocal(glyphOffset, vertex.cx, vertex.cy);
			else // For a straight line (STBTT_vline), the second point is in-between the start and end points
				curve.p2 = (curve.p1 + curve.p3) * 0.5f;
		
			curves.push_back(curve);
			curve.first = false;
		}
	}
	
	const float bandDelta = 0.0f;
	const uint32_t bandsTexelIndex = static_cast<uint32_t>(m_bandsTextureBandOffsets.size()) / 2;

	// Fix curves where the control point (p2) is one of the end points (p1 or p3).
	for (Curve& curve : curves)
	{
		if (curve.p1 == curve.p2 || curve.p2 == curve.p3)
			curve.p2 = (curve.p1 + curve.p3) * 0.5f;
	}
	
	// Start a transaction to rollback texture data in the case of a failure
	GlyphTransaction transaction(m_curvesTexture, m_bandsTextureBandOffsets, m_bandsTextureCurveOffsets);
	
	// Write the curve texture
	for (Curve& curve : curves)
	{
		if (curve.first && m_curvesTexture.size() % 4 != 0)
		{
			const size_t toAdd = 4 - (m_curvesTexture.size() % 4);
			for (size_t i = 0; i < toAdd; ++i)
			{
				m_curvesTexture.push_back(-1.0f);
			}
		}
		
		const bool newRow = (m_curvesTexture.size() / 4) % TEXTURE_WIDTH == TEXTURE_WIDTH - 1;
		if (newRow)
		{
			const size_t toAdd = 8 - (m_curvesTexture.size() % 4 );
			for (size_t i = 0; i < toAdd; ++i)
			{
				m_curvesTexture.push_back(-1.0f);
			}
		}
		
		if (curve.first || newRow)
		{
			curve.texelIndex = static_cast<uint32_t>(m_curvesTexture.size()) / 4;
			assert(m_curvesTexture.size() % 4 == 0); // TODO: Check possible improvement here
			m_curvesTexture.push_back(curve.p1.x);
			m_curvesTexture.push_back(curve.p1.y);
		}
		else
		{
			// Curves that are connected reuse the preceding end point as their start point.
			// Thus, we can simply determine which texel contains this shared end point.
			curve.texelIndex = ((static_cast<uint32_t>(m_curvesTexture.size()) / 2) - 1) / 2;
		}
		
		assert(m_curvesTexture.size() % 2 == 0);
		m_curvesTexture.push_back(curve.p2.x);
		m_curvesTexture.push_back(curve.p2.y);
		m_curvesTexture.push_back(curve.p3.x);
		m_curvesTexture.push_back(curve.p3.y);
	}
	
	uint32_t sizeX = 1 + static_cast<uint32_t>(bounds.x2 - bounds.x1);
	uint32_t sizeY = 1 + static_cast<uint32_t>(bounds.y2 - bounds.y1);
	
	uint32_t bandCount = m_bandCount;
	if (sizeX < bandCount || sizeY < bandCount)
		bandCount = std::min(sizeX, sizeY) / 2;
	
	if (bandCount == 0)
	{
		Logger::PrintError("Glyph U+{:04X} is too small for band generation.", p_codePoint);
		return false;
	}
	
	// Sort the curves based on their largest x-value
	// This is done to stop testing curves once suitable
	std::ranges::stable_sort(
		std::begin(curves),
		std::end(curves),
		[](const Curve& a, const Curve& b)
		{
			return	std::max({a.p1.x, a.p2.x, a.p3.x}) >
					std::max({b.p1.x, b.p2.x, b.p3.x});
	});

	// Calculate the dimension of the band, and the y-value of the top and bottom boundaries 
	const uint32_t bandDimY = (sizeY + bandCount - 1) / bandCount; // How wide is one band in the y-axis?
	float bandMinY = -bandDelta;
	float bandMaxY = static_cast<float>(bandDimY) + bandDelta;

	// Generate the horizontal bands
	for (uint32_t b = 0; b < bandCount; ++b)
	{
		uint16_t bandTexelOffset = static_cast<uint16_t>(m_bandsTextureCurveOffsets.size()) / 2;
		uint16_t curveCount = 0; 
		
		for (const Curve& curve : curves)
		{
			// Reject perfectly horizontal curves.
			if (curve.p1.y == curve.p2.y && curve.p2.y == curve.p3.y)
				continue;
			
			// Reject curves that are wholly outside the band's edges
			const float curveMinY = std::min({curve.p1.y, curve.p2.y, curve.p3.y});
			const float curveMaxY = std::max({curve.p1.y, curve.p2.y, curve.p3.y});
			if (curveMinY > bandMaxY || curveMaxY < bandMinY)
				continue;
			
			const uint32_t texelIndex = curve.texelIndex;
			const uint16_t curveOffsetX = static_cast<uint16_t>(texelIndex % TEXTURE_WIDTH);
			const uint16_t curveOffsetY = static_cast<uint16_t>(texelIndex / TEXTURE_WIDTH);
			m_bandsTextureCurveOffsets.push_back(curveOffsetX);
			m_bandsTextureCurveOffsets.push_back(curveOffsetY);
			
			++curveCount;
		}
		
		m_bandsTextureBandOffsets.push_back(curveCount);
		m_bandsTextureBandOffsets.push_back(bandTexelOffset);
		
		// Advance the band boundaries
		bandMinY += static_cast<float>(bandDimY);
		bandMaxY += static_cast<float>(bandDimY);
		
		if (bandTexelOffset >= 0xFFFF ||
			m_bandsTextureCurveOffsets.size() / 2 >= 0xFFFF)
		{
			Logger::PrintError("Too much data generated to be indexed. Try a lower band count.");
			return false;
		}
	}
	
	// Sort the curves based on their largest y-value
	std::ranges::stable_sort(
		std::begin(curves),
		std::end(curves),
		[](const Curve& a, const Curve& b)
		{
			return	std::max({a.p1.y, a.p2.y, a.p3.y}) >
					std::max({b.p1.y, b.p2.y, b.p3.y});
	});
	
	// Generate the vertical bands
	const uint32_t bandDimX = (sizeX + bandCount - 1) / bandCount;
	float bandMinX = -bandDelta;
	float bandMaxX = static_cast<float>(bandDimX) + bandDelta;
	
	for (uint32_t b = 0; b < bandCount; ++b)
	{
		uint16_t bandTexelOffset = static_cast<uint16_t>(m_bandsTextureCurveOffsets.size()) / 2;
		uint16_t curveCount = 0; 
		
		for (const Curve& curve : curves)
		{
			// Reject perfectly horizontal curves.
			if (curve.p1.x == curve.p2.x && curve.p2.x == curve.p3.x)
				continue;
			
			// Reject curves that are wholly outside the band's edges
			const float curveMinX = std::min({curve.p1.x, curve.p2.x, curve.p3.x});
			const float curveMaxX = std::max({curve.p1.x, curve.p2.x, curve.p3.x});
			if (curveMinX > bandMaxX || curveMaxX < bandMinX)
				continue;
			
			const uint32_t texelIndex = curve.texelIndex;
			const uint16_t curveOffsetX = static_cast<uint16_t>(texelIndex % TEXTURE_WIDTH);
			const uint16_t curveOffsetY = static_cast<uint16_t>(texelIndex / TEXTURE_WIDTH);
			m_bandsTextureCurveOffsets.push_back(curveOffsetX);
			m_bandsTextureCurveOffsets.push_back(curveOffsetY);
			
			++curveCount;
		}
		
		m_bandsTextureBandOffsets.push_back(curveCount);
		m_bandsTextureBandOffsets.push_back(bandTexelOffset);
		
		// Advance the band boundaries
		bandMinX += static_cast<float>(bandDimX);
		bandMaxX += static_cast<float>(bandDimX);
		
		if (bandTexelOffset >= 0xFFFF ||
			m_bandsTextureCurveOffsets.size() / 2 >= 0xFFFF)
		{
			Logger::PrintError("Too much data generated to be indexed. Try a lower band count.");
			return false;
		}
	}
	
	// Save the code point
	SlugCodePoint cp;
	cp.codePoint = p_codePoint;
	cp.width = static_cast<uint32_t>(bounds.x2 - bounds.x1);
	cp.height = static_cast<uint32_t>(bounds.y2 - bounds.y1);
	cp.bandCount = bandCount;
	cp.bandDimX = bandDimX;
	cp.bandDimY = bandDimY;
	cp.bandsTexCoordX = static_cast<uint16_t>(bandsTexelIndex % TEXTURE_WIDTH);
	cp.bandsTexCoordY = static_cast<uint16_t>(bandsTexelIndex / TEXTURE_WIDTH);
	m_codePoints.push_back(cp);
	
	if (bandsTexelIndex / TEXTURE_WIDTH >= 0xFFFF)
	{
		Logger::PrintError("Too much curve data generated.");
		return false;
	}
	
	// Verify codepoint data
	// We later assume that a single curve's data is present on one row only. If the curve data is present across several
	// rows, this assumption will fail.
	for (const Curve& c : curves)
	{
		const bool sameRow = c.texelIndex / TEXTURE_WIDTH == (c.texelIndex + 1) / TEXTURE_WIDTH;
		if (!sameRow)
			Logger::PrintError("Encoding failed! Texel indices {} and {} are not in the same row.", c.texelIndex, c.texelIndex + 1);
	}
	
	transaction.Commit();
	return true;
}

bool Droplet::Devtools::SlugGenerator::FontProcessor::SaveData(const std::filesystem::path& p_outPath)
{
	// TODO: Handle endianness correctly
	if (m_codePoints.empty())
	{
		Logger::PrintError("No valid code point found.");
		return false;
	}
	
	const uint32_t bandsTexTexels = static_cast<uint32_t>(m_bandsTextureBandOffsets.size() + m_bandsTextureCurveOffsets.size() / 2);
	const uint16_t bandHeaderTexels = static_cast<uint16_t>(m_bandsTextureBandOffsets.size() / 2);
	for (size_t i = 0; i < m_bandsTextureBandOffsets.size(); i += 2)
	{
		const uint32_t curveCount = m_bandsTextureBandOffsets[i];
		const uint16_t adjustedOffset = m_bandsTextureBandOffsets[i+1] + bandHeaderTexels;
		
		if (adjustedOffset > UINT16_MAX ||
			adjustedOffset > bandsTexTexels ||
			curveCount > bandsTexTexels - adjustedOffset)
		{
			Logger::PrintError("Invalid band offset or count.");
			return false;
		}
		
		m_bandsTextureBandOffsets[i+1] = static_cast<uint16_t>(adjustedOffset);
	}
	
	File
	
	return true;
}

bool Droplet::Devtools::SlugGenerator::FontProcessor::Error(const std::string &p_err)
{
	m_err = p_err;
	m_isValid = false;
	return false;
}

Droplet::Devtools::SlugGenerator::GlyphBounds Droplet::Devtools::SlugGenerator::FontProcessor::GetGlyphBounds(int p_glyphIndex) const
{
	GlyphBounds bounds;
    stbtt_GetGlyphBox(&m_fontInfo, p_glyphIndex, &bounds.x1, &bounds.y1, &bounds.x2, &bounds.y2);
	return bounds;
}

glm::vec2 Droplet::Devtools::SlugGenerator::FontProcessor::ToGlyphLocal(const glm::vec2 &p_glyphOffset, const float p_x,
	const float p_y)
{
	return {p_x - p_glyphOffset.x, p_y - p_glyphOffset.y};
}
