#ifndef VADON_CORE_FILE_DATABASE_HPP
#define VADON_CORE_FILE_DATABASE_HPP
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <Vadon/Utilities/System/UUID/UUID.hpp>
namespace Vadon::Core
{
	enum class FileDatabaseType
	{
		INVALID,
		FILESYSTEM, // i.e "on disk"
		// TODO: any other kinds of DB?
	};

	struct FileDatabaseInfo
	{
		FileDatabaseType type;
		std::string root_path;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(FileDatabase, FileDatabaseHandle);
}
#endif