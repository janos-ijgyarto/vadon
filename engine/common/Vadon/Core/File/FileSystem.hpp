#ifndef VADON_CORE_FILE_FILESYSTEM_HPP
#define VADON_CORE_FILE_FILESYSTEM_HPP
#include <Vadon/Core/CoreModule.hpp>
namespace Vadon::Core
{
	// FIXME: implement file mapping API!
	class FileSystem : public CoreSystem<FileSystem>
	{
	public:
		using RawFileDataBuffer = std::vector<std::byte>;

		virtual std::string get_absolute_path(std::string_view relative_path) const = 0;
		virtual std::vector<std::string> get_files_of_type(std::string_view path = "", std::string_view type = "", bool recursive = false) const = 0;
		
		virtual size_t get_file_size(std::string_view relative_path) const = 0;
		virtual bool does_file_exist(std::string_view relative_path) const = 0;

		// TODO: revise this? Implement caching, etc?
		// TODO2: allow setting how file is processed (binary, etc)?
		virtual bool save_file(std::string_view relative_path, const RawFileDataBuffer& file_data) = 0;
		virtual bool load_file(std::string_view relative_path, RawFileDataBuffer& file_data) = 0;
	protected:
		FileSystem(EngineCoreInterface& core)
			: System(core)
		{
		}
	};
}
#endif