#pragma once
#include <filesystem>
#include <string>
#include <fstream>
#include <vector>

namespace Droplet::Devtools::SlugGenerator
{
	/// @brief Recursively finds files with the specified extension.
	///
	/// Searches the provided directory and all its subdirectories
	/// for regular files matching the specified extension.
	///
	/// @param directory Directory to search.
	/// @param extension File extension to match, including the dot (e.g. ".ttf").
	/// @return Paths to all matching files, or an empty vector if none
	///         are found or the directory is invalid.
	std::vector<std::filesystem::path> GetFilesWithExtension(const std::filesystem::path& directory, const std::string& extension);
	
	/// @brief Provides basic binary file reading and writing operations.
	///
	/// Wraps std::fstream to simplify file access and error handling
	/// during Slug font generation.
	///
	/// Files are opened in binary read mode by default.
	/// The underlying stream is automatically closed on destruction.
	class File
	{
	public:
		/// @brief Constructs a file object and attempts to open the file.
		///
		/// @param p_path Path to the file.
		/// @param p_mode File opening mode. Defaults to binary reading.
		///
		/// @note Use IsOpen() to determine whether opening succeeded.
		explicit File(const std::filesystem::path& p_path, std::fstream::openmode p_mode = std::fstream::in | std::fstream::binary);
		
		~File() = default; // fstream auto-closes when destroyed
		
		/// @brief Checks whether the underlying file stream is open.
		/// @return true if the file is open, false otherwise.
		[[nodiscard]] bool IsOpen() const { return m_file.is_open(); }
		
		/// @brief Returns the cached size of the file.
		///
		/// The size is determined when a file is opened for reading.
		/// It is not updated after writing or external file modifications.
		///
		/// @return Cached file size in bytes, or zero if no size was recorded.
		[[nodiscard]] size_t GetSize() const { return m_fileSize; };
		
		/// @brief Reads the entire file into a byte buffer.
		///
		/// Resizes the destination vector to match the cached file size,
		/// resets the stream position to the beginning, and reads the data.
		///
		/// @param p_dst Destination vector receiving the file contents.
		/// @return true if the entire file was read successfully,
		///         false otherwise.
		///
		/// @note The file must be open for reading.
		bool ReadAll(std::vector<char>& p_dst);
		
		/// @brief Writes raw bytes to the file.
		///
		/// Writes the specified number of bytes from the provided memory
		/// address at the current output position.
		///
		/// @param p_data Pointer to the data to write.
		/// @param p_size Number of bytes to write.
		/// @return true if the write operation succeeded, false otherwise.
		///
		/// @note The file must be open for writing.
		/// @note Data is written in its native memory representation.
		///       No endianness conversion or serialization is performed.
		bool WriteBytes(const void* p_data, const size_t p_size);
		
		/// @brief Opens a file using the specified mode.
		///
		/// @param p_path Path to the file.
		/// @param p_mode File opening mode. Defaults to binary reading.
		/// @return true if the file was opened successfully, false otherwise.
		///
		/// @note An already open file must be closed before opening another.
		bool Open(const std::filesystem::path &p_path, std::fstream::openmode p_mode = std::fstream::in | std::fstream::binary);
		
		/// @brief Closes the currently open file.
		///
		/// Closes the underlying stream, flushing any buffered output.
		/// Does nothing if the file is already closed.
		///
		/// @return true if the file was closed successfully or was already
		///         closed, false if closing failed.
		///
		/// @note For writing operations, check the return value to detect
		///       errors that may only become apparent when closing the stream.
		bool Close();
		
	private:
		std::fstream m_file;
		std::filesystem::path m_filePath;
		
		/// @brief Cached file size in bytes, determined when opening.
		size_t m_fileSize = 0;
	};
}
