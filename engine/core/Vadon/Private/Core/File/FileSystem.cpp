#include <Vadon/Private/PCH/Core.hpp>
#include <Vadon/Private/Core/File/FileSystem.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <format>
#include <fstream>

namespace
{
	std::filesystem::path get_generic_path(std::string_view path)
	{
		return std::filesystem::path(path).generic_string();
	}

	void file_database_serialization_error_log()
	{
		Vadon::Core::Logger::log_error("File system: failed to serialize database!\n");
	}
}

namespace Vadon::Private::Core
{
	const FileInfo* FileSystem::FileDatabaseData::find_file(const FileID& file_id) const
	{
		auto file_it = file_lookup.find(file_id);
		if (file_it != file_lookup.end())
		{
			return &file_it->second;
		}

		return nullptr;
	}

	FileDatabaseHandle FileSystem::create_database(const FileDatabaseInfo& info)
	{
		VADON_ASSERT(info.type != FileDatabaseType::INVALID, "Invalid database type!");

		const FileDatabaseHandle new_db_handle = m_database_pool.add();
		
		FileDatabaseData& new_db_data = m_database_pool.get(new_db_handle);
		new_db_data.info = info;

		// Make sure path is generic format
		new_db_data.root_path = get_generic_path(info.root_path);

		return new_db_handle;
	}

	FileDatabaseInfo FileSystem::get_database_info(FileDatabaseHandle db_handle) const
	{
		const FileDatabaseData& database_data = m_database_pool.get(db_handle);
		return database_data.info;
	}

	void FileSystem::remove_database(FileDatabaseHandle db_handle)
	{
		m_database_pool.remove(db_handle);
	}

	bool FileSystem::save_database(FileDatabaseHandle db_handle, RawFileDataBuffer& db_data)
	{
		FileDatabaseData& database_data = m_database_pool.get(db_handle);

		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(db_data, Vadon::Utilities::Serializer::Type::BINARY, Vadon::Utilities::Serializer::Mode::WRITE);
		return serialize_database(*serializer, database_data);
	}

	bool FileSystem::load_database(FileDatabaseHandle db_handle, RawFileDataBuffer& db_data)
	{
		FileDatabaseData& database_data = m_database_pool.get(db_handle);

		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(db_data, Vadon::Utilities::Serializer::Type::BINARY, Vadon::Utilities::Serializer::Mode::READ);
		return serialize_database(*serializer, database_data);
	}

	std::string FileSystem::get_absolute_path(FileDatabaseHandle db_handle, std::string_view path) const
	{
		const std::filesystem::path fs_path = get_generic_path(path);
		return internal_get_absolute_path(db_handle, path).generic_string();
	}

	std::string FileSystem::get_relative_path(FileDatabaseHandle db_handle, std::string_view path) const
	{
		const std::filesystem::path fs_path = get_generic_path(path);
		return internal_get_relative_path(db_handle, fs_path).generic_string();
	}

	FileID FileSystem::add_file(FileDatabaseHandle db_handle, const FileInfo& info)
	{
		FileDatabaseData& database_data = m_database_pool.get(db_handle);

		for (const auto& file_info_pair : database_data.file_lookup)
		{
			if (file_info_pair.second.path == info.path)
			{
				// TODO: warn that path already exists?
				return FileID();
			}
		}

		std::filesystem::path fs_path = get_generic_path(info.path);
		if (fs_path.is_absolute() == true)
		{
			fs_path = internal_get_relative_path(db_handle, fs_path);
		}

		FileID added_file_id;
		while (added_file_id.is_valid() == false)
		{
			if (database_data.file_lookup.find(added_file_id) != database_data.file_lookup.end())
			{
				added_file_id.invalidate();
			}
		}

		database_data.file_lookup.insert(std::make_pair(added_file_id, info));
		return added_file_id;
	}

	bool FileSystem::add_existing_file(FileDatabaseHandle db_handle, const FileID& file_id, const FileInfo& info)
	{
		FileDatabaseData& database_data = m_database_pool.get(db_handle);

		if (database_data.file_lookup.find(file_id) != database_data.file_lookup.end())
		{
			// TODO: error?
			return false;
		}

		database_data.file_lookup.insert(std::make_pair(file_id, info));
		return true;
	}

	FileInfo FileSystem::get_file_info(FileDatabaseHandle db_handle, const FileID& file_id) const
	{
		FileInfo file_info;

		const FileDatabaseData& database_data = m_database_pool.get(db_handle);
		const FileInfo* db_file_info = database_data.find_file(file_id);
		if (db_file_info != nullptr)
		{
			file_info = *db_file_info;
		}

		return file_info;
	}

	bool FileSystem::does_file_exist(FileDatabaseHandle db_handle, const FileID& file_id) const
	{
		const FileDatabaseData& database_data = m_database_pool.get(db_handle);
		return database_data.find_file(file_id) != nullptr;		
	}

	void FileSystem::remove_file(FileDatabaseHandle db_handle, const FileID& file_id)
	{
		FileDatabaseData& database_data = m_database_pool.get(db_handle);
		database_data.file_lookup.erase(file_id);
	}

	std::vector<FileID> FileSystem::get_all_files(FileDatabaseHandle db_handle) const
	{
		std::vector<FileID> result;
		const FileDatabaseData& database_data = m_database_pool.get(db_handle);

		for (const auto& file_pair : database_data.file_lookup)
		{
			result.push_back(file_pair.first);
		}

		return result;
	}

	bool FileSystem::save_file(FileDatabaseHandle db_handle, const FileID& file_id, const RawFileDataBuffer& file_data)
	{
		// TODO: implement for other DB types!
		const FileDatabaseData& database_data = m_database_pool.get(db_handle);
		if (database_data.info.type != FileDatabaseType::FILESYSTEM)
		{
			// TODO: log error!
			return false;
		}

		const FileInfo* db_file_info = database_data.find_file(file_id);
		if (db_file_info == nullptr)
		{
			// TODO: log error!
			return false;
		}

		const std::filesystem::path absolute_path = get_absolute_path(db_handle, db_file_info->path);
		return internal_save_file(absolute_path, file_data);
	}

	bool FileSystem::save_file(std::string_view absolute_path, const RawFileDataBuffer& file_data)
	{
		const std::filesystem::path fs_absolute_path = get_generic_path(absolute_path);
		return internal_save_file(fs_absolute_path, file_data);
	}

	bool FileSystem::load_file(FileDatabaseHandle db_handle, const FileID& file_id, RawFileDataBuffer& file_data)
	{
		// TODO: implement for other DB types!
		const FileDatabaseData& database_data = m_database_pool.get(db_handle);
		if (database_data.info.type != FileDatabaseType::FILESYSTEM)
		{
			// TODO: log error!
			return false;
		}

		const FileInfo* db_file_info = database_data.find_file(file_id);
		if (db_file_info == nullptr)
		{
			// TODO: log error!
			return false;
		}

		const std::filesystem::path absolute_path = get_absolute_path(db_handle, db_file_info->path);
		return internal_load_file(absolute_path, file_data);
	}

	bool FileSystem::load_file(std::string_view absolute_path, RawFileDataBuffer& file_data)
	{
		const std::filesystem::path fs_absolute_path = get_generic_path(absolute_path);
		return internal_load_file(fs_absolute_path, file_data);
	}

	bool FileSystem::copy_file(FileDatabaseHandle source_db, const FileID& source_file, FileDatabaseHandle dest_db, const FileID& dest_file)
	{
		// TODO: implement for other DB types!
		const FileDatabaseData& source_db_data = m_database_pool.get(source_db);
		const FileDatabaseData& dest_db_data = m_database_pool.get(dest_db);
		if ((source_db_data.info.type != FileDatabaseType::FILESYSTEM) || (dest_db_data.info.type != FileDatabaseType::FILESYSTEM))
		{
			// TODO: log error!
			return false;
		}

		const FileInfo* source_file_info = source_db_data.find_file(source_file);
		if (source_file_info == nullptr)
		{
			// TODO: log error!
			return false;
		}

		const FileInfo* dest_file_info = dest_db_data.find_file(dest_file);
		if (dest_file_info == nullptr)
		{
			// TODO: log error!
			return false;
		}

		const std::filesystem::path src_absolute_path = get_absolute_path(source_db, source_file_info->path);
		if (std::filesystem::exists(src_absolute_path) != true)
		{
			log_error(std::format("File system error: file \"{}\" does not exist!\n", src_absolute_path.generic_string()));
			return false;
		}

		const std::filesystem::path dest_absolute_path = get_absolute_path(dest_db, dest_file_info->path);

		// Make sure parent path exists
		std::error_code fs_error;
		const std::filesystem::path dest_parent_path = dest_absolute_path.parent_path();
		if (std::filesystem::exists(dest_parent_path) == false)
		{
			std::filesystem::create_directories(dest_parent_path, fs_error);
			if (fs_error)
			{
				// TODO: log the specific error?
				log_error(std::format("File system error: unable to create path \"{}\"!\nError: \"{}\"\n", dest_parent_path.generic_string(), fs_error.message()));
				return false;
			}
		}

		// TODO: make use of return value?
		std::filesystem::copy_file(src_absolute_path, dest_absolute_path, std::filesystem::copy_options::overwrite_existing, fs_error);

		if (fs_error)
		{
			// TODO: log the specific error?
			log_error(std::format("File system error: unable to copy file \"{}\" to \"{}\"!\nError: \"{}\"\n", src_absolute_path.generic_string(), dest_absolute_path.generic_string(), fs_error.message()));
			return false;
		}

		return true;
	}

	FileSystem::FileSystem(Vadon::Core::EngineCoreInterface& core)
		: Vadon::Core::FileSystem(core)
	{

	}

	bool FileSystem::initialize()
	{
		// TODO: anything?
		return true;
	}

	std::filesystem::path FileSystem::internal_get_absolute_path(FileDatabaseHandle db_handle, const std::filesystem::path& path) const
	{
		// Append to root path
		const FileDatabaseData& database_data = m_database_pool.get(db_handle);
		return (database_data.root_path / path);
	}

	std::filesystem::path FileSystem::internal_get_relative_path(FileDatabaseHandle db_handle, const std::filesystem::path& path) const
	{
		const FileDatabaseData& database_data = m_database_pool.get(db_handle);
		return std::filesystem::relative(path, database_data.root_path);
	}

	size_t FileSystem::internal_get_file_size(const std::filesystem::path& absolute_path) const
	{
		try
		{
			const auto file_size = std::filesystem::file_size(absolute_path);
			return static_cast<size_t>(file_size);
		}
		catch (std::filesystem::filesystem_error& e)
		{
			log_error(std::format("File system error: \"{}\"\n", e.what()));
		}
		return 0;
	}

	bool FileSystem::internal_save_file(const std::filesystem::path& path, const RawFileDataBuffer& file_data)
	{
		// Create directories as necessary
		const std::filesystem::path parent_path = path.parent_path();
		std::error_code fs_error;
		if (std::filesystem::create_directories(parent_path, fs_error) == false)
		{
			// Check error code
			if (fs_error)
			{
				// TODO: log the specific error?
				log_error(std::format("File system error: unable to create path to file \"{}\"!\n", path.generic_string()));
				return false;
			}
		}

		std::ofstream file_stream(path, std::ios::binary);
		if (file_stream.good() == false)
		{
			log_error(std::format("File system error: unable to write file \"{}\"!\n", path.generic_string()));
			return false;
		}

		file_stream.write(reinterpret_cast<const char*>(file_data.data()), file_data.size());
		file_stream.close();

		return true;
	}

	bool FileSystem::internal_load_file(const std::filesystem::path& path, RawFileDataBuffer& file_data)
	{
		if (std::filesystem::exists(path) != true)
		{
			log_error(std::format("File system error: file \"{}\" does not exist!\n", path.generic_string()));
			return false;
		}

		const size_t file_size = internal_get_file_size(path);
		std::ifstream file_stream(path, std::ios::binary);
		if (file_stream.good() == false)
		{
			log_error(std::format("File system error: unable to read file \"{}\"!\n", path.generic_string()));
			return false;
		}

		const size_t buffer_prev_size = file_data.size();
		file_data.resize(buffer_prev_size + file_size);

		file_stream.read(reinterpret_cast<char*>(file_data.data() + buffer_prev_size), file_size);
		file_stream.close();

		return true;
	}

	bool FileSystem::serialize_database(Vadon::Utilities::Serializer& serializer, FileDatabaseData& database)
	{
		if (serializer.initialize() == false)
		{
			log_error("File system: unable to initialize serializer for database!\n");
			return false;
		}

		using SerializerResult = Vadon::Utilities::Serializer::Result;

		// Serialize any custom properties to a separate object
		if (serializer.open_array("files") != SerializerResult::SUCCESSFUL)
		{
			file_database_serialization_error_log();
			return false;
		}

		if (serializer.is_reading() == true)
		{
			database.file_lookup.clear();

			const size_t file_count = serializer.get_array_size();
			for (size_t current_file_index = 0; current_file_index < file_count; current_file_index)
			{
				if (serializer.open_object(current_file_index) != SerializerResult::SUCCESSFUL)
				{
					file_database_serialization_error_log();
					return false;
				}

				FileID file_id;
				if (serializer.serialize("id", file_id) != SerializerResult::SUCCESSFUL)
				{
					file_database_serialization_error_log();
					return false;
				}

				FileInfo file_info;
				{
					if (serializer.serialize("path", file_info.path) != SerializerResult::SUCCESSFUL)
					{
						file_database_serialization_error_log();
						return false;
					}

					if (serializer.serialize("offset", file_info.offset) != SerializerResult::SUCCESSFUL)
					{
						file_database_serialization_error_log();
						return false;
					}

					if (serializer.serialize("size", file_info.size) != SerializerResult::SUCCESSFUL)
					{
						file_database_serialization_error_log();
						return false;
					}
				}

				database.file_lookup.insert(std::make_pair(file_id, file_info));

				if (serializer.close_object() != SerializerResult::SUCCESSFUL)
				{
					file_database_serialization_error_log();
					return false;
				}
			}
		}
		else
		{
			size_t file_index = 0;
			for (auto& file_pair : database.file_lookup)
			{
				if (serializer.open_object(file_index) != SerializerResult::SUCCESSFUL)
				{
					file_database_serialization_error_log();
					return false;
				}

				{
					FileID file_id = file_pair.first;
					if (serializer.serialize("id", file_id) != SerializerResult::SUCCESSFUL)
					{
						file_database_serialization_error_log();
						return false;
					}
				}

				FileInfo& file_info = file_pair.second;
				{
					if (serializer.serialize("path", file_info.path) != SerializerResult::SUCCESSFUL)
					{
						file_database_serialization_error_log();
						return false;
					}

					if (serializer.serialize("offset", file_info.offset) != SerializerResult::SUCCESSFUL)
					{
						file_database_serialization_error_log();
						return false;
					}

					if (serializer.serialize("size", file_info.size) != SerializerResult::SUCCESSFUL)
					{
						file_database_serialization_error_log();
						return false;
					}
				}

				if (serializer.close_object() != SerializerResult::SUCCESSFUL)
				{
					file_database_serialization_error_log();
					return false;
				}

				++file_index;
			}
		}		

		if (serializer.close_array() != SerializerResult::SUCCESSFUL)
		{
			file_database_serialization_error_log();
			return false;
		}

		if (serializer.finalize() == false)
		{
			log_error("File system: failed to finalize serializer for database!\n");
			return false;
		}

		return true;
	}
}