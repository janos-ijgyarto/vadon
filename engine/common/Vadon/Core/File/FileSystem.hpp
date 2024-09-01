#ifndef VADON_CORE_FILE_FILESYSTEM_HPP
#define VADON_CORE_FILE_FILESYSTEM_HPP
#include <Vadon/Core/CoreModule.hpp>
#include <Vadon/Core/File/RootDirectory.hpp>
namespace Vadon::Core
{
	// FIXME: implement file mapping API!
	class FileSystem : public CoreSystem<FileSystem>
	{
	public:
		struct Path
		{
			std::string_view path;
			RootDirectoryHandle root;
		};

		using RawFileDataBuffer = std::vector<std::byte>;

		virtual RootDirectoryHandle add_root_directory(RootDirectoryInfo info) = 0;
		virtual RootDirectoryInfo get_root_info(RootDirectoryHandle root_handle) const = 0;
		virtual void remove_root_directory(RootDirectoryHandle root_handle) = 0;

		virtual std::string get_absolute_path(Path path) const = 0;
		virtual std::vector<std::string> get_files_of_type(Path path, std::string_view type, bool recursive = false) const = 0;
		
		virtual size_t get_file_size(Path path) const = 0;
		virtual bool does_file_exist(Path path) const = 0;

		// TODO: revise this? Implement caching, etc?
		// TODO2: allow setting how file is processed (binary, etc)?
		virtual bool save_file(Path path, const RawFileDataBuffer& file_data) = 0;
		virtual bool load_file(Path path, RawFileDataBuffer& file_data) = 0;
	protected:
		FileSystem(EngineCoreInterface& core)
			: System(core)
		{
		}
	};
}
#endif