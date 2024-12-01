#ifndef VADON_CORE_FILE_PATH_HPP
#define VADON_CORE_FILE_PATH_HPP
#include <Vadon/Core/File/RootDirectory.hpp>
namespace Vadon::Core
{
	struct FileSystemPath
	{
		RootDirectoryHandle root_directory;
		std::string path;

		bool is_valid() const { return (root_directory.is_valid()) || (path.empty() == false); }
	};
}
#endif