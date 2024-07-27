#ifndef VADON_CORE_FILE_FILESYSTEM_HPP
#define VADON_CORE_FILE_FILESYSTEM_HPP
#include <Vadon/Core/CoreModule.hpp>
namespace Vadon::Core
{
	class FileSystem : public CoreSystem<FileSystem>
	{
	public:
		using RawFileDataBuffer = std::vector<std::byte>;

		virtual std::string get_absolute_path(std::string_view relative_path) const = 0;
		virtual std::vector<std::string> get_files_of_type(std::string_view path = "", std::string_view type = "", bool recursive = false) const = 0;
		
		// TODO: revise this? Implement caching, etc?
		// TODO2: allow setting how file is loaded (binary, etc)?
		virtual bool load_file(std::string_view relative_path, RawFileDataBuffer& file_data) = 0;
	protected:
		FileSystem(EngineCoreInterface& core)
			: System(core)
		{
		}
	};
}
#endif