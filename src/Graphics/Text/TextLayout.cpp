#include "Text/TextLayout.hpp"

Droplet::Graphics::Text::TextLayoutResult Droplet::Graphics::Text::BuildTextLayout(const FontResource &p_font,
	const std::string_view &p_text, float p_emSize, const glm::vec4 &p_color)
{
	
}

std::vector<std::uint32_t> Droplet::Graphics::Text::DecodeUTF8(const std::string_view &p_text)
{
	std::vector<std::uint32_t> codePoints;

	for (std::size_t i = 0; i < p_text.size();)
	{
		const auto first = static_cast<std::uint8_t>(p_text[i]);

		std::uint32_t codePoint = 0;
		std::size_t length = 0;

		if (first < 0x80)
		{
			codePoint = first;
			length = 1;
		}
		else if ((first & 0xE0) == 0xC0)
		{
			codePoint = first & 0x1F;
			length = 2;
		}
		else if ((first & 0xF0) == 0xE0)
		{
			codePoint = first & 0x0F;
			length = 3;
		}
		else if ((first & 0xF8) == 0xF0)
		{
			codePoint = first & 0x07;
			length = 4;
		}
		else
		{
			throw std::runtime_error("Invalid UTF-8");
		}

		if (i + length > p_text.size())
			throw std::runtime_error("Incomplete UTF-8 sequence");

		for (std::size_t j = 1; j < length; ++j)
		{
			const auto byte = static_cast<std::uint8_t>(p_text[i + j]);

			if ((byte & 0xC0) != 0x80)
				throw std::runtime_error("Invalid UTF-8 continuation byte");

			codePoint = (codePoint << 6) | (byte & 0x3F);
		}

		if ((length == 2 && codePoint < 0x80) ||
			(length == 3 && codePoint < 0x800) ||
			(length == 4 && codePoint < 0x10000) ||
			(codePoint >= 0xD800 && codePoint <= 0xDFFF) ||
			codePoint > 0x10FFFF)
		{
			throw std::runtime_error("Invalid UTF-8 code point");
		}

		codePoints.push_back(codePoint);
		i += length;
	}

	return codePoints;
}
