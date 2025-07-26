#ifndef VADON_PRIVATE_CORE_FILE_FILESYSTEM_HPP
#define VADON_PRIVATE_CORE_FILE_FILESYSTEM_HPP
#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/Private/Core/File/Database.hpp>
#include <Vadon/Private/Core/File/File.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>

#include <filesystem>
#include <unordered_map>

namespace Vadon::Utilities
{
	class Serializer;
}

namespace Vadon::Private::Core
{
	class FileSystem : public Vadon::Core::FileSystem
	{
	public:
		FileDatabaseHandle create_database(const FileDatabaseInfo& info) override;
		FileDatabaseInfo get_database_info(FileDatabaseHandle db_handle) const override;
		void remove_database(FileDatabaseHandle db_handle) override;

		bool save_database(FileDatabaseHandle db_handle, RawFileDataBuffer& db_data) override;
		bool load_database(FileDatabaseHandle db_handle, RawFileDataBuffer& db_data) override;

		std::string get_absolute_path(FileDatabaseHandle db_handle, std::string_view path) const override;
		std::string get_relative_path(FileDatabaseHandle db_handle, std::string_view path) const override;

		FileID add_file(FileDatabaseHandle db_handle, const FileInfo& info) override;
		bool add_existing_file(FileDatabaseHandle db_handle, const FileID& file_id, const FileInfo& info) override;
		FileInfo get_file_info(FileDatabaseHandle db_handle, const FileID& file_id) const override;
		bool does_file_exist(FileDatabaseHandle db_handle, const FileID& file_id) const override;
		void remove_file(FileDatabaseHandle db_handle, const FileID& file_id) override;

		std::vector<FileID> get_all_files(FileDatabaseHandle db_handle) const override;
				
		bool save_file(FileDatabaseHandle db_handle, const FileID& file_id, const RawFileDataBuffer& file_data) override;
		bool save_file(std::string_view absolute_path, const RawFileDataBuffer& file_data) override;

		bool load_file(FileDatabaseHandle db_handle, const FileID& file_id, RawFileDataBuffer& file_data) override;
		bool load_file(std::string_view absolute_path, RawFileDataBuffer& file_data) override;

		bool copy_file(FileDatabaseHandle source_db, const FileID& source_file, FileDatabaseHandle dest_db, const FileID& dest_file) override;

		std::string get_current_path() const override { return std::filesystem::current_path().generic_string(); }
	protected:
		// TODO: have some way to create DB subclasses that can implement save/load/copy/etc.
		// This will enable interactions between DBs without needing to know the backend
		struct FileDatabaseData
		{
			FileDatabaseInfo info;
			std::filesystem::path root_path;
			std::unordered_map<FileID, FileInfo> file_lookup;
			// TODO: anything else?

			const FileInfo* find_file(const FileID& file_id) const;
		};

		FileSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();

		std::filesystem::path internal_get_absolute_path(FileDatabaseHandle db_handle, const std::filesystem::path& path) const;
		std::filesystem::path internal_get_relative_path(FileDatabaseHandle db_handle, const std::filesystem::path& path) const;

		size_t internal_get_file_size(const std::filesystem::path& absolute_path) const;

		bool internal_save_file(const std::filesystem::path& path, const RawFileDataBuffer& file_data);
		bool internal_load_file(const std::filesystem::path& path, RawFileDataBuffer& file_data);

		bool serialize_database(Vadon::Utilities::Serializer& serializer, FileDatabaseData& database);

		Vadon::Utilities::ObjectPool<Vadon::Core::FileDatabase, FileDatabaseData> m_database_pool;

		friend class EngineCore;
	};
}
#endif