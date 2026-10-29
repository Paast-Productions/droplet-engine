#include <Stb/stb_truetype.h>
#include <filesystem>
#include <vector>

#include "File.hpp"
#include "FontProcessor.hpp"
#include "Logger.hpp"

using namespace Droplet::Devtools::SlugGenerator;

int main()
{
	// Get all .ttf files from the Assets/Fonts directory.
	// TODO: Add command-line option to only generate binary files for fonts which don't have a corresponding binary, and another for generating only for a specific file.
	std::filesystem::path fontPath =  std::filesystem::canonical("../../../Assets/Fonts");
	std::vector<std::filesystem::path> sourceFiles = GetFilesWithExtension(fontPath, ".ttf");

	// This is only done for clarity, and may be changed if a different destination is desired.
	// TODO: Add command-line option to define a different destination.
	std::filesystem::path destinationPath = fontPath;
	
	for (const auto& sourceFile : sourceFiles)
	{
		FontProcessor font(sourceFile);
		if (!font.IsValid())
		{
			Logger::PrintError("Failed to process font at {} - {}", sourceFile.string(), font.GetError());
			// continue;
		}
	}
	
	return 0;
}
