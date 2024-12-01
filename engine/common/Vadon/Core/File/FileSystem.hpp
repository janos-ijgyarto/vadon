#ifndef VADON_CORE_FILE_FILESYSTEM_HPP
#define VADON_CORE_FILE_FILESYSTEM_HPP
#include <Vadon/Core/CoreModule.hpp>
#include <Vadon/Core/File/Path.hpp>
namespace Vadon::Core
{
	// FIXME: implement file mapping API!
	class FileSystem : public CoreSystem<FileSystem>
	{
	public:
		using RawFileDataBuffer = std::vector<std::byte>;

		virtual RootDirectoryHandle add_root_directory(const RootDirectoryInfo& info) = 0;
		virtual RootDirectoryInfo get_root_info(RootDirectoryHandle root_handle) const = 0;
		virtual void remove_root_directory(RootDirectoryHandle root_handle) = 0;

		virtual RootDirectoryHandle get_default_root() const = 0;

		virtual std::string get_absolute_path(const FileSystemPath& path) const = 0;
		virtual std::string get_relative_path(std::string_view path, RootDirectoryHandle root_handle = RootDirectoryHandle()) const = 0;
		virtual std::vector<FileSystemPath> get_files_of_type(const FileSystemPath& root_path, std::string_view type, bool recursive = false) const = 0;
		
		virtual size_t get_file_size(const FileSystemPath& path) const = 0;
		virtual bool does_file_exist(const FileSystemPath& path) const = 0;

		// TODO: revise this? Implement caching, etc?
		// TODO2: allow setting how file is processed (binary, etc)?
		virtual bool save_file(const FileSystemPath& path, const RawFileDataBuffer& file_data) = 0;
		virtual bool load_file(const FileSystemPath& path, RawFileDataBuffer& file_data) = 0;
	protected:
		FileSystem(EngineCoreInterface& core)
			: System(core)
		{
		}
	};
}
#endif