#ifndef VADON_CORE_FILE_FILE_HPP
#define VADON_CORE_FILE_FILE_HPP
#include <Vadon/Foundation/Utilities/UUID.hpp>
#include <string>
#include <vector>
namespace Vadon::Core
{
	using FileID = ::Vadon::Foundation::UUID;

	struct FileInfo
	{
		std::string path;
		// TODO: ensure that valid offset and size are always stored?
		int offset = 0;
		int size = 0;

		bool is_valid() const
		{
			return path.empty() == false;
		}
	};

	using RawFileDataBuffer = std::vector<std::byte>;
}
#endif