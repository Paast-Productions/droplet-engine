#pragma once
#include <Stb/stb_truetype.h>

namespace Droplet::Devtools::SlugGenerator::FontProcessing
{
	bool ProcessCodePoint(stbtt_fontinfo& p_fontinfo, int p_codePoint);
}
