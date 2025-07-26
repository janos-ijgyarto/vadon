#ifndef VADON_CORE_FILE_FILESYSTEM_HPP
#define VADON_CORE_FILE_FILESYSTEM_HPP
#include <Vadon/Core/CoreModule.hpp>
#include <Vadon/Core/File/File.hpp>
#include <Vadon/Core/File/Database.hpp>
namespace Vadon::Core
{
	// NOTE: this is a "virtual" file system
	// It is only aware of files that were explicitly registered to it
	class FileSystem : public CoreSystem<FileSystem>
	{
	public:
		virtual FileDatabaseHandle create_database(const FileDatabaseInfo& info) = 0;
		virtual FileDatabaseInfo get_database_info(FileDatabaseHandle db_handle) const = 0;
		virtual void remove_database(FileDatabaseHandle db_handle) = 0;

		virtual bool save_database(FileDatabaseHandle db_handle, RawFileDataBuffer& db_data) = 0;
		virtual bool load_database(FileDatabaseHandle db_handle, RawFileDataBuffer& db_data) = 0;

		// NOTE: the system does not validate whether the inputs are valid!
		// It will assume a valid input and append to the root path or create a relative path from root!
		virtual std::string get_absolute_path(FileDatabaseHandle db_handle, std::string_view path) const = 0;
		virtual std::string get_relative_path(FileDatabaseHandle db_handle, std::string_view path) const = 0;

		virtual FileID add_file(FileDatabaseHandle db_handle, const FileInfo& info) = 0;
		virtual bool add_existing_file(FileDatabaseHandle db_handle, const FileID& file_id, const FileInfo& info) = 0;
		virtual FileInfo get_file_info(FileDatabaseHandle db_handle, const FileID& file_id) const = 0;
		virtual bool does_file_exist(FileDatabaseHandle db_handle, const FileID& file_id) const = 0;
		virtual void remove_file(FileDatabaseHandle db_handle, const FileID& file_id) = 0;

		virtual std::vector<FileID> get_all_files(FileDatabaseHandle db_handle) const = 0;

		// TODO: implement a file I/O interface that can take advantage of OS-specific file I/O features
		// The default implementation can just be the current one, i.e save/load an entire buffer at once
		virtual bool save_file(FileDatabaseHandle db_handle, const FileID& file_id, const RawFileDataBuffer& file_data) = 0;
		virtual bool save_file(std::string_view absolute_path, const RawFileDataBuffer& file_data) = 0;

		virtual bool load_file(FileDatabaseHandle db_handle, const FileID& file_id, RawFileDataBuffer& file_data) = 0;
		virtual bool load_file(std::string_view absolute_path, RawFileDataBuffer& file_data) = 0;

		virtual bool copy_file(FileDatabaseHandle source_db, const FileID& source_file, FileDatabaseHandle dest_db, const FileID& dest_file) = 0;

		// FIXME: this is meant to be something set by client code, for the time being we'll just return std::filesystem::current_path
		virtual std::string get_current_path() const = 0;
	protected:
		FileSystem(EngineCoreInterface& core)
			: System(core)
		{
		}
	};
}
#endif