#pragma once
#include <vector>
#include "SlugHelper.hpp"

namespace Droplet::Devtools::SlugGenerator
{
	/// @brief Provides automatic rollback of glyph data during processing.
	///
	/// Records the initial sizes of the shared curve texture, band texture
	/// arrays, and code-point metadata. If processing ends without calling
	/// Commit(), all appended data is discarded.
	///
	/// @note Only appended data is restored. Modifications to existing
	///       elements cannot be rolled back.
	/// @note The referenced vectors must outlive the transaction.
	class GlyphTransaction
	{
	public:
		/// @brief Begins a transaction for the shared glyph data.
		///
		/// Records the current sizes of all supplied vectors without
		/// copying their contents.
		///
		/// @param p_curves Packed Bézier curve texture data.
		/// @param p_bandOffsets Band headers containing curve counts and offsets.
		/// @param p_curveOffsets Curve references within the band texture.
		/// @param p_codePoints Metadata for processed code points.
		GlyphTransaction(
			std::vector<float>& p_curves,
			std::vector<uint16_t>& p_bandOffsets,
			std::vector<uint16_t>& p_curveOffsets,
			std::vector<SlugCodePoint>& p_codePoints)
				:	m_curves(&p_curves),
					m_bandOffsets(&p_bandOffsets),
					m_curveOffsets(&p_curveOffsets),
					m_codePoints(&p_codePoints),
					m_curvesSize(p_curves.size()),
					m_bandOffsetsSize(p_bandOffsets.size()),
					m_curveOffsetsSize(p_curveOffsets.size()),
					m_codePointsSize(p_codePoints.size())
		{}
		
		/// @brief Rolls back appended data unless the transaction was committed.
		///
		/// Restores all vector sizes recorded during construction,
		/// discarding any elements appended during the transaction.
		~GlyphTransaction()
		{
			// Discard added data in case we exit without committing (e.g. in case of a failure)
			if (!m_committed)
			{
				m_curves->resize(m_curvesSize);
				m_bandOffsets->resize(m_bandOffsetsSize);
				m_curveOffsets->resize(m_curveOffsetsSize);
				m_codePoints->resize(m_codePointsSize);
			}
		}
		
		/// @brief Commits the transaction, retaining all appended data.
		///
		/// Prevents the destructor from restoring the original vector sizes.
		void Commit() noexcept
		{
			m_committed = true;
		}
		
		GlyphTransaction(const GlyphTransaction&) = delete;
		GlyphTransaction& operator=(const GlyphTransaction&) = delete;
		GlyphTransaction(GlyphTransaction&&) = delete;
		GlyphTransaction& operator=(GlyphTransaction&&) = delete;
			
	private:
		std::vector<float>* m_curves;
		std::vector<uint16_t>* m_bandOffsets;
		std::vector<uint16_t>* m_curveOffsets;
		std::vector<SlugCodePoint>* m_codePoints;

		size_t m_curvesSize;
		size_t m_bandOffsetsSize;
		size_t m_curveOffsetsSize;
		size_t m_codePointsSize;

		bool m_committed = false;
	};
}
