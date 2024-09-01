#ifndef VADON_CORE_FILE_ROOTDIRECTORY_HPP
#define VADON_CORE_FILE_ROOTDIRECTORY_HPP
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
namespace Vadon::Core
{
	struct RootDirectoryInfo
	{
		std::string path;
		// TODO: any other settings
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(RootDirectory, RootDirectoryHandle);
}
#endif