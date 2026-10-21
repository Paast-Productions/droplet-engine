#include <Stb/stb_truetype.h>
#include <filesystem>
#include <vector>

#include "File.hpp"
#include "FontProcessing.hpp"
#include "Logger.hpp"

using namespace Droplet::Devtools::SlugGenerator;

int main()
{
	// Get all .ttf files from the Assets/Fonts directory.
	// TODO: Add command-line option to only generate binary files for fonts which don't have a corresponding binary, and another for generating only for a specific file.
	std::filesystem::path fontPath =  std::filesystem::canonical("../../../Assets/Fonts");
	std::vector<std::filesystem::path> sourceFiles = Droplet::Devtools::SlugGenerator::GetFilesWithExtension(fontPath, ".ttf");

	// This is only done for clarity, and may be changed if a different destination is desired.
	// TODO: Add command-line option to define a different destination.
	std::filesystem::path destinationPath = fontPath;
	
	for (const auto& sourceFile : sourceFiles)
	{
		// Open the source font file
		File file(sourceFile);
		if (!file.IsOpen())
			continue;
		
		// This must be kept alive until the stbtt_fontinfo further down is no longer used.
		std::vector<char> fontData;
		if (!file.ReadAll(fontData))
		{
			// The file is closed upon class destruction.
			Logger::PrintError("Failed to read font data.");
			return 1;
		}
		file.Close(); // The file is no longer needed.
		
		// Get the font info
		static stbtt_fontinfo font;
		if (!stbtt_InitFont(&font, reinterpret_cast<const unsigned char *>(fontData.data()), 0))
		{
			return 1;
		}
		
		// The range of ascii characters to process (inclusive)
		// TODO: Support for extended ascii codes, such as the Swedish alphabet. This will require some change in how we loop below.
		constexpr int asciiRangeStart = 33;
		constexpr int asciiRangeEnd = 126;
		for (int codePoint = asciiRangeStart; codePoint <= asciiRangeEnd; ++codePoint)
		{
			// FontProcessing::ProcessCodePoint(font, codePoint);
		}
	}
	
	return 0;
}
