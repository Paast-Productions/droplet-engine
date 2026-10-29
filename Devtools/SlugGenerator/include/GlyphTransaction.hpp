#pragma once
#include <vector>

namespace Droplet::Devtools::SlugGenerator
{
	class GlyphTransaction
	{
	public:
		GlyphTransaction(
			std::vector<float>& p_curves,
			std::vector<uint16_t>& p_bandOffsets,
			std::vector<uint16_t>& p_curveOffsets)
				:	m_curves(&p_curves),
					m_bandOffsets(&p_bandOffsets),
					m_curveOffsets(&p_curveOffsets),
					m_curvesSize(p_curves.size()),
					m_bandOffsetsSize(p_bandOffsets.size()),
					m_curveOffsetsSize(p_curveOffsets.size())
		{}
		
		~GlyphTransaction()
		{
			// Discard added data in case we exit without committing (e.g. in case of a failure)
			if (!m_committed)
			{
				m_curves->resize(m_curvesSize);
				m_bandOffsets->resize(m_bandOffsetsSize);
				m_curveOffsets->resize(m_curveOffsetsSize);
			}
		}
		
		void Commit() noexcept
		{
			m_committed = true;
		}
		
		GlyphTransaction(const GlyphTransaction&) = delete;
		GlyphTransaction& operator=(const GlyphTransaction&) = delete;
		GlyphTransaction(GlyphTransaction&&) = delete;
		GlyphTransaction& operator=(const GlyphTransaction&&) = delete;
			
	private:
		std::vector<float>* m_curves;
		std::vector<uint16_t>* m_bandOffsets;
		std::vector<uint16_t>* m_curveOffsets;

		size_t m_curvesSize;
		size_t m_bandOffsetsSize;
		size_t m_curveOffsetsSize;

		bool m_committed = false;
	};
}
