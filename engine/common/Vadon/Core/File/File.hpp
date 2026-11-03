#ifndef VADON_CORE_FILE_FILE_HPP
#define VADON_CORE_FILE_FILE_HPP
#include <Vadon/Foundation/Utilities/UUID.hpp>
#include <chrono>
#include <string>
#include <vector>
namespace Vadon::Core
{
	using FileID = ::Vadon::Foundation::UUID;
	using FileTimeType = std::chrono::time_point<std::chrono::file_clock>; // FIXME: use int64_t so we don't have to include <chrono>!

	struct FileMetadata
	{
		bool exists = false;
		FileTimeType last_write_time; // Time since epoch
		::Vadon::Foundation::uint64 size = 0; // Size in bytes
	};

	struct FileInfo
	{
		std::string path;
		// TODO: ensure that valid offset and size are always stored?
		int offset = 0;
		int size = 0;
		FileMetadata metadata;

		bool is_valid() const
		{
			return path.empty() == false;
		}
	};

	using RawFileDataBuffer = std::vector<std::byte>;
}
#endif