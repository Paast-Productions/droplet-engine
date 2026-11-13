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
	std::filesystem::path fontPath =  "../../../../Assets/Fonts";
	if (!std::filesystem::is_directory(fontPath))
	{
		Logger::PrintError("Font directory does not exist.");
		if (!std::filesystem::create_directory(fontPath))
			Logger::PrintWarning("Could not create Font directory at Assets/Fonts/");
		else
			Logger::PrintInfo("Created Fonts directory at Assets/Fonts/ -  Put TrueType fonts in this directory and re-run the generator.");
		return 1;
	}
	
	std::vector<std::filesystem::path> sourceFiles = GetFilesWithExtension(fontPath, ".ttf");
	if (sourceFiles.empty())
	{
		Logger::PrintWarning("No TrueType fonts found.");
		return 1;
	}

	// This is only done for clarity, and may be changed if a different destination is desired.
	// TODO: Add command-line option to define a different destination.
	std::filesystem::path destinationDir = fontPath;
	
	for (const auto& sourceFile : sourceFiles)
	{
		FontProcessor font(sourceFile, destinationDir);
		if (!font.IsValid())
		{
			Logger::PrintError("Failed to process font at {} - {}", sourceFile.string(), font.GetError());
		}
	}
	
	return 0;
}
