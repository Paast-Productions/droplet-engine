#include "File.hpp"
#include "Logger.hpp"

std::vector<std::filesystem::path> Droplet::Devtools::SlugGenerator::GetFilesWithExtension(
	std::filesystem::path directory, std::string extension)
{
	std::vector<std::filesystem::path> paths;
	
	if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory))
	{
		Logger::PrintError("Could not get files with {} extension, source path {} is not a directory.", extension, directory.string());
		return paths;
	}
	
	Logger::PrintInfo("Searching for files with {} extension in {}...", extension, directory.string());
	
	for (const auto& entry : std::filesystem::recursive_directory_iterator(directory))
	{
		if (std::filesystem::is_regular_file(entry) && entry.path().extension() == extension)
			paths.emplace_back(entry.path());
	}
	
	Logger::PrintInfo("Found {} files.", paths.size());
	return paths;
}

Droplet::Devtools::SlugGenerator::File::File(const std::filesystem::path &p_path, std::fstream::openmode p_mode) : m_filePath(p_path)
{
	Open(p_path, p_mode); // Open() handles error logging, so we don't care about the result of the open here.
}

size_t Droplet::Devtools::SlugGenerator::File::GetSize() const
{
	return m_fileSize;
}

bool Droplet::Devtools::SlugGenerator::File::ReadAll(std::vector<char>& p_dst)
{
	// Don't open a new file if we already have one open.
	if (!IsOpen())
	{
		Logger::PrintError("Could not read file data. File at {} is not open.", m_filePath.string());
		return false;
	}
	
	// Don't try to read more than what is possible with fstream.
	if (m_fileSize > std::numeric_limits<std::streamsize>::max())
	{
		Logger::PrintError("Trying to read more data than fstream can handle ({}).", p_dst.size());
		return false;
	}
	
	// Resize the destination buffer to fit the data.
	p_dst.resize(m_fileSize);
	
	// Reset the file state
	m_file.clear();
	m_file.seekg(0, std::fstream::beg);
	
	// Read the file.
	if (!m_file.read(p_dst.data(), static_cast<std::streamsize>(p_dst.size())))
	{
		Logger::PrintError("Failed reading file at {}", m_filePath.string());
		return false;
	}
	
	return true;
}

bool Droplet::Devtools::SlugGenerator::File::Open(const std::filesystem::path &p_path, std::fstream::openmode p_mode)
{
	if (IsOpen())
	{
		Logger::PrintError("A file is already open.");
		return false;
	}
	
	if (!std::filesystem::exists(p_path))
	{
		Logger::PrintError("Could not open file at {}. File does not exist.", m_filePath.string());
		return false;
	}
	
	m_file.open(m_filePath, p_mode);
	if (!IsOpen())
	{
		Logger::PrintError("Could not open file at {}.", m_filePath.string());
		return false;
	}
	
	m_filePath = p_path;
	m_fileSize = std::filesystem::file_size(m_filePath); // Cache the file size to reduce filesystem calls
	return true;
}

void Droplet::Devtools::SlugGenerator::File::Close()
{
	if (IsOpen())
		m_file.close();
}
