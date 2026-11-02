#pragma once
#include <filesystem>
#include <string>
#include <fstream>
#include <vector>

namespace Droplet::Devtools::SlugGenerator
{
	std::vector<std::filesystem::path> GetFilesWithExtension(const std::filesystem::path& directory, const std::string& extension);
	
	class File
	{
	public:
		explicit File(const std::filesystem::path& p_path, std::fstream::openmode p_mode = std::fstream::in | std::fstream::binary);
		~File() = default; // fstream auto-closes when destroyed
		
		[[nodiscard]] bool IsOpen() const { return m_file.is_open(); }
		[[nodiscard]] size_t GetSize() const { return m_fileSize; };
		bool ReadAll(std::vector<char>& p_dst);
		bool WriteBytes(const void* p_data, const size_t p_size);
		
		bool Open(const std::filesystem::path &p_path, std::fstream::openmode p_mode = std::fstream::in | std::fstream::binary);
		bool Close();
		
	private:
		std::fstream m_file;
		std::filesystem::path m_filePath;
		size_t m_fileSize = 0;
	};
}
