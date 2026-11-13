// FontProcessor.cpp - Droplet Engine
//
// Based on the Sluggish font generator by mightycow:
// https://github.com/mightycow/Sluggish
//
// Original code released under Unlicense.
// Modified for use with the Droplet Engine.
// 
// See ../THIRD_PARTY_NOTICES.md for additional information

#include "FontProcessor.hpp"
#include "Logger.hpp"
#include "File.hpp"
#include "GlyphTransaction.hpp"

#include <Stb/stb_truetype.h>

#include "SlugHelper.hpp"

Droplet::Devtools::SlugGenerator::FontProcessor::FontProcessor(const std::filesystem::path &p_path, const std::filesystem::path &p_destination, const int p_asciiRangeStart, const int p_asciiRangeEnd, const uint32_t p_bandCount)
	: m_path(p_path), m_bandCount(p_bandCount)
{
	if (p_bandCount == 0)
	{
		Error("Band count must be greater than zero.");
		return;
	}
	
	// TODO: Check over the failure cases. When should we fatally fail etc?
	m_isValid = false;
	Logger::PrintInfo("Processing {}...", m_path.filename().string());
	
	if (!OpenFont())
		return;
	
	// The range of ascii characters to process (inclusive)
	// TODO: Support for extended ascii codes, such as the Swedish alphabet. This will require some change in how we loop below.
	int successfullyProcessed = 0;
	std::string missingChars{};
	for (int codePoint = p_asciiRangeStart; codePoint <= p_asciiRangeEnd; ++codePoint)
	{
		if (!ProcessCodePoint(codePoint))
		{
			const std::string miss = std::to_string(codePoint) + "(" + static_cast<char>(codePoint) + ") "; 
			missingChars += miss;
			continue;
		}
		
		++successfullyProcessed;
	}
	
	Logger::PrintInfo("Finished processing {}.", m_path.filename().string());
	
	int totalCount = p_asciiRangeEnd - p_asciiRangeStart + 1;
	Logger::PrintInfo("{} / {} code points successfully processed. ", successfullyProcessed, totalCount);
	
	if (successfullyProcessed != totalCount)
		Logger::PrintWarning("Missing characters {}", missingChars);
	
	// The file will be output in the p_destination directory. The file name will match the input .ttf, and the extension will be .slug
	std::filesystem::path outname = p_path.filename();
	outname.replace_extension(".slug");
	std::filesystem::path dest = p_destination / outname;
	
	if (!SaveData(dest))
	{
		Error("Could not save font data.");
		return;
	}
	
	Logger::PrintInfo("Successfully saved font data to {}.", dest.string());
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
		
	if (!stbtt_InitFont(&m_fontInfo, reinterpret_cast<const unsigned char *>(m_fontData.data()), 0))
		return Error("Could not get font information.");
	
	return true;
}

bool Droplet::Devtools::SlugGenerator::FontProcessor::ProcessCodePoint(const int p_codePoint)
{
	// TODO: Evaluate storing curve control points as 16-bit floats instead of 32-bit floats.
	// This is recommended by Slug, but is not done now for simplicity. Evaluate performance and accuracy before switching.
	
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
		// TODO: Add bicubic Bézier curve support
		if (vertices.get()[v].type == STBTT_vcubic)
		{
			Logger::PrintError("Code point {} uses bicubic Bézier curves, which are not supported currently.", p_codePoint);
			return false;
		}
	}
	
	GlyphBounds glyphBounds = GetGlyphBounds(glyphIdx);
	
	const glm::vec2 glyphOffset = { glyphBounds.GetMinX(), glyphBounds.GetMinY() };
	std::vector<Curve> curves = ExtractCurves(vertices.get(), vertexCount, glyphOffset);
	
	// TODO: Evaluate whether bandDelta should be non-zero.
	// bandDelta adds a margin to the curve selection, which may prove useful in some cases. Setting this to non-zero should be followed by thorough testing.
	const float bandDelta = 0.0f;
	const uint32_t bandsTexelIndex = static_cast<uint32_t>(m_bandsTextureBandOffsets.size()) / 2;

	
	// Start a transaction to rollback texture data in the case of a failure
	GlyphTransaction transaction(m_curvesTexture, m_bandsTextureBandOffsets, m_bandsTextureCurveOffsets, m_codePoints);
	
	// Write the curve texture
	for (Curve& curve : curves)
	{
		// The first curve of each contour must start on a texel boundary.
		// Pad the unused components with -1.
		if (curve.first && m_curvesTexture.size() % 4 != 0)
		{
			const size_t toAdd = 4 - (m_curvesTexture.size() % 4);
			for (size_t i = 0; i < toAdd; ++i)
			{
				m_curvesTexture.push_back(-1.0f);
			}
		}
		
		// We don't currently support two texels in separate rows.
		// A curve is later decoded	using two adjacent texels on the same row. Thus, if this curve's first texel is the
		// last one on this row, pad with -1 to start on a new row.
		const bool newRow = (m_curvesTexture.size() / 4) % TEXTURE_WIDTH == TEXTURE_WIDTH - 1;
		if (newRow)
		{
			const size_t toAdd = 8 - (m_curvesTexture.size() % 4 );
			for (size_t i = 0; i < toAdd; ++i)
			{
				m_curvesTexture.push_back(-1.0f);
			}
		}
		
		// Connected curves share endpoints to reduce storage.
		// Each bracket below represents one RGBA texel, and each point is a vec2
		// 
		// [P1 P2] [P3=(P1 of the next curve) P2] [P3=(P1 of the third curve) P2] ...
		//
		// curve.texelIndex represents the first texel to be used to decode the curve
		if (curve.first || newRow)
		{
			curve.texelIndex = static_cast<uint32_t>(m_curvesTexture.size()) / 4;
			
			assert(m_curvesTexture.size() % 4 == 0);
			
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
	
	// Calculate the glyph's dimensions
	uint32_t sizeX = 1 + static_cast<uint32_t>(glyphBounds.xMax - glyphBounds.xMin);
	uint32_t sizeY = 1 + static_cast<uint32_t>(glyphBounds.yMax - glyphBounds.yMin);
	
	uint32_t bandCount = m_bandCount;
	if (sizeX < bandCount || sizeY < bandCount)
		bandCount = std::min(sizeX, sizeY) / 2;
	
	if (bandCount == 0)
	{
		Logger::PrintError("Glyph U+{:04X} is too small for band generation.", p_codePoint);
		return false;
	}
	
	// Sort the curves based on their largest x-value, descending
	// The renderer can then use this fact to stop testing curves once the remaining curves cannot contribute to the
	// current sample.
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
		size_t bandTexelOffset = m_bandsTextureCurveOffsets.size() / 2;
		if (bandTexelOffset > UINT16_MAX)
		{
			Logger::PrintError("Band reference offset exceeds 16-bit range.");
			return false;
		}
		
		size_t curveCount = 0;
		
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
			
			// Store the curve's offset in the texture
			const uint32_t texelIndex = curve.texelIndex;
			const uint16_t curveOffsetX = static_cast<uint16_t>(texelIndex % TEXTURE_WIDTH);
			const uint16_t curveOffsetY = static_cast<uint16_t>(texelIndex / TEXTURE_WIDTH);
			m_bandsTextureCurveOffsets.push_back(curveOffsetX);
			m_bandsTextureCurveOffsets.push_back(curveOffsetY);
			
			++curveCount;
		}
		
		// There is an optimization possible here. If this band is the same as the previous one, we can skip adding the
		// data to the list and simply reuse the reference index. This could save some space.
		// TODO: Optimize to remove duplicate data entries in the band texture.
		
		if (curveCount > UINT16_MAX)
		{
			Logger::PrintError("Curve count exceeds 16-bit range.");
			return false;
		}
		m_bandsTextureBandOffsets.push_back(static_cast<uint16_t>(curveCount));
		
		const uint16_t storedOffset = static_cast<uint16_t>(bandTexelOffset);
		m_bandsTextureBandOffsets.push_back(storedOffset);
		
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
	
	// Sort the curves based on their largest y-value, descending
	// This allows the renderer to stop intersection tests early.
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
		size_t bandTexelOffset = m_bandsTextureCurveOffsets.size() / 2;
		if (bandTexelOffset > UINT16_MAX)
		{
			Logger::PrintError("Band reference offset exceeds 16-bit range.");
			return false;
		}
		
		size_t curveCount = 0;
		
		for (const Curve& curve : curves)
		{
			// Reject perfectly vertical curves.
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
		
		m_bandsTextureBandOffsets.push_back(static_cast<uint16_t>(curveCount));
		
		const uint16_t storedOffset = static_cast<uint16_t>(bandTexelOffset);
		m_bandsTextureBandOffsets.push_back(storedOffset);
		
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
	
	int advanceWidth, leftSideBearing;
	stbtt_GetGlyphHMetrics(&m_fontInfo, glyphIdx, &advanceWidth, &leftSideBearing);
	
	const float scaleFactor = stbtt_ScaleForMappingEmToPixels(&m_fontInfo, 1.0f);
	const uint16_t unitsPerEm = static_cast<uint16_t>(std::lround(1.0f / scaleFactor));
	
	// Store font metadata
	m_fontHeader.unitsPerEm = unitsPerEm;
	
	// Store glyph metadata
	SlugCodePoint cp;
	cp.codePoint = p_codePoint;
	cp.width = static_cast<uint32_t>(glyphBounds.xMax - glyphBounds.xMin);
	cp.height = static_cast<uint32_t>(glyphBounds.yMax - glyphBounds.yMin);
	cp.advanceWidth = advanceWidth;
	cp.xMin = glyphBounds.xMin;
	cp.yMin = glyphBounds.yMin;
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
		if (c.texelIndex / TEXTURE_WIDTH
			!=
			(c.texelIndex + 1) / TEXTURE_WIDTH) // Since we can store a curve's three points in two texels, this is enough to check the assumption
		{
			Logger::PrintError("Encoding failed! Texel indices {} and {} are not in the same row.", c.texelIndex, c.texelIndex + 1);
			return false;
		}
		
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
	
	// In the final band texture, band headers precede the curve references.
	// Thus, we should adjust each band's reference offset to account for this.
	// TODO: Move this adjustment to a once-per-font execution - if SaveData() is called again on the same font, the offsets will break.
	const size_t bandsTexTexels = (m_bandsTextureBandOffsets.size() + m_bandsTextureCurveOffsets.size()) / 2;
	const size_t bandHeaderTexels = m_bandsTextureBandOffsets.size() / 2;
	for (size_t i = 0; i < m_bandsTextureBandOffsets.size(); i += 2)
	{
		const size_t curveCount = m_bandsTextureBandOffsets[i];
		const size_t adjustedOffset = m_bandsTextureBandOffsets[i+1] + bandHeaderTexels;
		
		// Validate the curve-reference range.
		// An empty band may point one past the final texel
		if (adjustedOffset > UINT16_MAX ||
			adjustedOffset > bandsTexTexels ||
			curveCount > bandsTexTexels - adjustedOffset)
		{
			Logger::PrintError("Invalid band offset or count.");
			return false;
		}
		
		m_bandsTextureBandOffsets[i+1] = static_cast<uint16_t>(adjustedOffset);
	}
	
	
	// Write the font data to the file
	File outFile(p_outPath, std::ios::binary | std::ios::out);
	if (!outFile.IsOpen())
	{
		Logger::PrintError("Could not create or open output file at {}.", p_outPath.string());
		return false;
	}
	
	if (!outFile.WriteBytes(SLUG_HEADER_DATA, SLUG_HEADER_LEN)) return false;
	
	if (!outFile.WriteBytes(&m_fontHeader, sizeof(SlugFontHeader))) return false;
	
	const uint16_t codePointCount = static_cast<uint16_t>(m_codePoints.size());
	if (!outFile.WriteBytes(&codePointCount, sizeof(codePointCount))) return false;
	if (!outFile.WriteBytes(m_codePoints.data(), codePointCount * sizeof(SlugCodePoint))) return false;
	
	const uint32_t curvesTexTexels = m_curvesTexture.size() / 4;
	const uint32_t curvesTexBytes = m_curvesTexture.size() * sizeof(m_curvesTexture.front());
	const uint16_t curvesTexWidth = TEXTURE_WIDTH;
	const uint16_t curvesTexHeight = (curvesTexTexels + curvesTexWidth - 1) / curvesTexWidth;
	
	if (!outFile.WriteBytes(&curvesTexWidth, sizeof(curvesTexWidth))) return false;
	if (!outFile.WriteBytes(&curvesTexHeight, sizeof(curvesTexWidth))) return false;
	if (!outFile.WriteBytes(&curvesTexBytes, sizeof(curvesTexBytes))) return false;
	if (!outFile.WriteBytes(m_curvesTexture.data(), curvesTexBytes)) return false;
	
	const uint16_t bandsTexWidth = TEXTURE_WIDTH;
	const uint16_t bandsTexHeight = (bandsTexTexels + bandsTexWidth - 1) / bandsTexWidth;
	const uint32_t bandsTexBytes = bandsTexTexels * sizeof(uint16_t) * 2;
	
	if (!outFile.WriteBytes(&bandsTexWidth, sizeof(bandsTexWidth))) return false;
	if (!outFile.WriteBytes(&bandsTexHeight, sizeof(bandsTexHeight))) return false;
	if (!outFile.WriteBytes(&bandsTexBytes, sizeof(bandsTexBytes))) return false;
	
	if (!outFile.WriteBytes(m_bandsTextureBandOffsets.data(), m_bandsTextureBandOffsets.size() * sizeof(m_bandsTextureBandOffsets.front()))) return false;
	if (!outFile.WriteBytes(m_bandsTextureCurveOffsets.data(), m_bandsTextureCurveOffsets.size() * sizeof(m_bandsTextureCurveOffsets.front()))) return false;
	
	return outFile.Close();
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
    stbtt_GetGlyphBox(&m_fontInfo, p_glyphIndex, &bounds.xMin, &bounds.yMin, &bounds.xMax, &bounds.yMax);
	return bounds;
}


/// @brief Translates a point from font coordinates to glyph-local coordinates.
///
/// Subtracts the glyph's minimum bounding-box coordinates from the given point,
/// placing the glyph's bounding-box origin at (0, 0).
///
/// @param p_glyphOffset The minimum X and Y coordinates of the glyph's bounding box.
/// @param p_x The original X coordinate.
/// @param p_y The original Y coordinate.
/// @return The translated point in glyph-local coordinates.
glm::vec2 Droplet::Devtools::SlugGenerator::FontProcessor::ToGlyphLocal(const glm::vec2 &p_glyphOffset, const float p_x,
	const float p_y)
{
	return {p_x - p_glyphOffset.x, p_y - p_glyphOffset.y};
}

std::vector<Droplet::Devtools::SlugGenerator::Curve> Droplet::Devtools::SlugGenerator::FontProcessor::ExtractCurves(
	const stbtt_vertex *p_vertices, const int p_numVerts, const glm::vec2 p_glyphOffset)
{
	std::vector<Curve> curves;
	curves.reserve(static_cast<size_t>(p_numVerts));
	
	Curve curve{};
	for (int v = 0; v < p_numVerts; ++v)
	{
		const stbtt_vertex& vertex = p_vertices[v];
		if (vertex.type == STBTT_vmove) // This vertex defines the starting point of a contour
		{
			// The starting point of a contour cannot share it's starting point with a previous contour.
			// Setting curve.first allows us to account for this when packing the data.
			curve.first = true;
			curve.p3 = ToGlyphLocal(p_glyphOffset, vertex.x, vertex.y);
			continue;
		}
	
		// Should not occur, but early out to be safe
		if (vertex.type != STBTT_vcurve && vertex.type != STBTT_vline)
		{
			Logger::PrintWarning("Encountered unexpected vertex type.");
			continue;
		}

		// The end of the previous curve makes out the start of the new one
		curve.p1 = curve.p3;
	
		curve.p3 = ToGlyphLocal(p_glyphOffset, vertex.x, vertex.y);
	
		if (vertex.type == STBTT_vcurve) // For a curve, translate the second point to the glyph bounds
			curve.p2 = ToGlyphLocal(p_glyphOffset, vertex.cx, vertex.cy);
		else // For a straight line (STBTT_vline), the second point is in-between the start and end points
			curve.p2 = (curve.p1 + curve.p3) * 0.5f;
	
		curves.push_back(curve);
		curve.first = false;
	}
	
	// Fix curves where the control point (p2) is one of the end points (p1 or p3).
	for (Curve& c : curves)
	{
		if (c.p1 == c.p2 || c.p2 == c.p3)
			c.p2 = (c.p1 + c.p3) * 0.5f;
	}
	
	return curves;
}
