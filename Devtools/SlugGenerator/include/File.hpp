#pragma once
#include <filesystem>
#include <string>
#include <fstream>
#include <span>
#include <vector>

namespace Droplet::Devtools::SlugGenerator
{
	std::vector<std::filesystem::path> GetFilesWithExtension(const std::filesystem::path& directory, const std::string& extension);
	
	class File
	{
	public:
		explicit File(const std::filesystem::path& p_path, std::fstream::openmode p_mode = std::fstream::in | std::fstream::binary);
		~File() = default; // fstream auto-closes when destroyed
		
		bool IsOpen() const { return m_file.is_open(); }
		size_t GetSize() const;
		bool ReadAll(std::vector<char>& p_dst);
		bool WriteBytes(const void* p_data, const size_t p_size);
		
		bool Open(const std::filesystem::path &p_path, std::fstream::openmode p_mode = std::fstream::in | std::fstream::binary);
		void Close();
		
	private:
		std::fstream m_file;
		std::filesystem::path m_filePath;
		size_t m_fileSize = 0;
	};
}
