#include <VadonEditor/Model/Resource/Database.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <Vadon/Model/Resource/File.hpp>
#include <Vadon/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

// FIXME: this is a quick hacky solution!
// Long-term we should replace nlohmann with simdjson since it can now both read and write documents!
#if defined(_MSC_VER)
# pragma warning(push)
# pragma warning(disable: 4100) // unreferenced formal parameter
# pragma warning(disable: 4244) // 'return': conversion from 'simdjson::error_code' to 'char', possible loss of data
#endif
#include <simdjson.h>
#if defined(_MSC_VER)
# pragma warning(pop)
#endif

#include <filesystem>

namespace
{
	std::string get_filesystem_style_asset_extension(::Vadon::Foundation::ResourceFileInfo::Type file_type)
	{
		return std::format(".{}", ::Vadon::Foundation::ResourceFileInfo::get_file_extension(file_type));
	}

	::Vadon::Foundation::ResourceFileInfo::Type get_file_asset_type(const std::filesystem::path& path)
	{
		const std::string extension = path.extension().generic_string();
		if (extension == get_filesystem_style_asset_extension(::Vadon::Foundation::ResourceFileInfo::Type::SCENE))
		{
			return ::Vadon::Foundation::ResourceFileInfo::Type::SCENE;
		}
		else if (extension == get_filesystem_style_asset_extension(::Vadon::Foundation::ResourceFileInfo::Type::RESOURCE))
		{
			return ::Vadon::Foundation::ResourceFileInfo::Type::RESOURCE;
		}
		else if (extension == get_filesystem_style_asset_extension(::Vadon::Foundation::ResourceFileInfo::Type::IMPORTED_FILE))
		{
			return ::Vadon::Foundation::ResourceFileInfo::Type::IMPORTED_FILE;
		}

		return ::Vadon::Foundation::ResourceFileInfo::Type::NONE;
	}

	std::string_view get_sanitized_json_key(std::string_view original_key)
	{
		const size_t separator_index = original_key.find('|');
		if (separator_index != std::string::npos)
		{
			return original_key.substr(separator_index + 1);
		}
		else
		{
			return original_key;
		}
	}

	bool recursive_sanitize_json(simdjson::ondemand::value element, simdjson::builder::string_builder& builder) {
		bool add_comma;
		switch (element.type()) {
		case simdjson::ondemand::json_type::array:
			builder.start_array();
			add_comma = false;
			for (auto child : element.get_array()) {
				if (add_comma) {
					builder.append_comma();
				}
				// We need the call to value() to get
				// an ondemand::value type.
				recursive_sanitize_json(child.value(), builder);
				add_comma = true;
			}
			builder.end_array();
			break;
		case simdjson::ondemand::json_type::object:
			builder.start_object();
			add_comma = false;
			for (auto field : element.get_object()) {
				if (add_comma) {
					builder.append_comma();
				}
				std::string_view key_string = field.escaped_key();
				std::string_view sanitized_key_string = get_sanitized_json_key(key_string);

				builder.append(sanitized_key_string);
				builder.append_colon();
				recursive_sanitize_json(field.value(), builder);
				add_comma = true;
			}
			builder.end_object();
			break;
		case simdjson::ondemand::json_type::number:
			// assume it fits in a double
			builder.append(element.get_double().value());
			break;
		case simdjson::ondemand::json_type::string:
			builder.append(element.get_string().value());
			break;
		case simdjson::ondemand::json_type::boolean:
			builder.append(element.get_bool().value());
			break;
		case simdjson::ondemand::json_type::null:
			// We check that the value is indeed null
			// otherwise: an error is thrown.
			if (element.is_null()) {
				builder.append_null();
			}
			break;
		case simdjson::ondemand::json_type::unknown:
			// TODO: error?
			return false;
		}

		return true;
	}

	bool simdjson_sanitize_json_data(Vadon::Core::RawFileDataBuffer& file_buffer)
	{
		if (file_buffer.empty() == true)
		{
			// Nothing to do
			return true;
		}

		// Add padding required by simdjson
		const size_t original_length = file_buffer.size();
		file_buffer.insert(file_buffer.end(), simdjson::SIMDJSON_PADDING, std::byte{ 0 });

		simdjson::padded_string_view simdjson_string(reinterpret_cast<char*>(file_buffer.data()), original_length, file_buffer.size());

		simdjson::builder::string_builder builder;

		simdjson::ondemand::parser parser;
		simdjson::ondemand::document doc = parser.iterate(simdjson_string);
		simdjson::ondemand::value val = doc;
		
		if (recursive_sanitize_json(val, builder) == false)
		{
			return false;
		}

		std::string_view builder_string = builder.view();

		file_buffer.resize(builder_string.size());
		memcpy(file_buffer.data(), builder_string.data(), builder_string.size());

		return true;
	}
}

namespace VadonEditor::Model
{
	ResourceDatabase::ResourceDatabase(Vadon::Core::EngineCoreInterface& engine_core, VadonEditor::Core::ProjectManager& project_manager)
		: m_engine_core(engine_core)
		, m_project_manager(project_manager)
	{

	}

	bool ResourceDatabase::initialize()
	{
		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();

		// Create the databases for resources and assets
		{
			Vadon::Core::FileDatabaseInfo resource_db_info;
			resource_db_info.root_path = m_project_manager.get_active_project().root_path;
			resource_db_info.type = Vadon::Core::FileDatabaseType::FILESYSTEM;

			m_file_databases[static_cast<size_t>(FileDatabaseType::RESOURCE)] = file_system.create_database(resource_db_info);
		}

		{
			Vadon::Core::FileDatabaseInfo asset_db_info;
			asset_db_info.root_path = m_project_manager.get_active_project().root_path;
			asset_db_info.type = Vadon::Core::FileDatabaseType::FILESYSTEM;

			m_file_databases[static_cast<size_t>(FileDatabaseType::ASSET_FILE)] = file_system.create_database(asset_db_info);
		}

		return true;
	}

	bool ResourceDatabase::save_resource(Vadon::Model::ResourceSystem& resource_system, Vadon::Model::ResourceHandle resource_handle)
	{
		const Vadon::Model::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);

		Vadon::Core::RawFileDataBuffer resource_file_buffer;
		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::WRITE);

		if (serializer->initialize() == false)
		{
			resource_system.log_error("Editor resource database: failed to initialize serializer while saving resource!\n");
			return false;
		}

		if (resource_system.save_resource(*serializer, resource_handle) == false)
		{
			resource_system.log_error("Editor resource database: failed to serialize resource data!\n");
			return false;
		}

		if (serializer->finalize() == false)
		{
			resource_system.log_error("Editor resource database: failed to finalize serializer after saving resource!\n");
			return false;
		}

		// Save via the asset library
		const Vadon::Core::FileDatabaseHandle resource_file_db = get_database(FileDatabaseType::RESOURCE);

		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();
		if (file_system.save_file(resource_file_db, resource_info.id, resource_file_buffer) == false)
		{
			resource_system.log_error("Editor resource database: failed to save resource data to file!\n");
			return false;
		}

		return true;
	}

	Vadon::Model::ResourceHandle ResourceDatabase::load_resource(Vadon::Model::ResourceSystem& resource_system, Vadon::Model::ResourceID resource_id)
	{
		const Vadon::Core::FileDatabaseHandle resource_file_db = get_database(FileDatabaseType::RESOURCE);

		// First make sure we have a valid file for this resource
		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();
		const Vadon::Core::FileInfo resource_file_info = file_system.get_file_info(resource_file_db, resource_id);
		if (resource_file_info.is_valid() == false)
		{
			resource_system.log_error("Editor resource database: failed to load resource file!\n");
			return Vadon::Model::ResourceHandle();
		}

		// Check whether a temp file is also available		
		std::filesystem::path temp_file_path = m_project_manager.get_active_project().root_path;
		temp_file_path /= ".vadon/temp/model";
		temp_file_path /= Vadon::Utilities::uuid_to_hex_string(resource_id) + ".vdtmp";

		Vadon::Core::RawFileDataBuffer resource_file_buffer;

		if (file_system.does_file_exist(temp_file_path.string()) == true)
		{
			// Temp file exists, check if it's more recent
			const Vadon::Core::FileMetadata temp_file_metadata = file_system.get_file_metadata(temp_file_path.string());
			if (temp_file_metadata.last_write_time > resource_file_info.metadata.last_write_time)
			{
				// Temp file was updated more recently, load data from there
				if (file_system.load_file(temp_file_path.string(), resource_file_buffer) == false)
				{
					resource_system.log_error("Editor resource database: failed to load temp resource file!\n");
				}
			}
		}

		if (resource_file_buffer.empty() == true)
		{
			// No temp file, load from original resource file
			if (file_system.load_file(resource_file_db, resource_id, resource_file_buffer) == false)
			{
				resource_system.log_error("Editor resource database: failed to load resource file!\n");
				return Vadon::Model::ResourceHandle();
			}

			// This is a "raw" editor file, we have to clear the labels from the keys
			if (sanitize_editor_resource_file(resource_file_buffer) == false)
			{
				resource_system.log_error("Editor resource database: failed to process resource file data!\n");
				return Vadon::Model::ResourceHandle();
			}
		}
		VADON_ASSERT(resource_file_buffer.empty() == false, "Failed to load resource data!");

		// FIXME: support binary file serialization!
		// Solution: have file system create the appropriate serializer!
		Vadon::Utilities::Serializer::Instance serializer_instance = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);

		if (serializer_instance->initialize() == false)
		{
			resource_system.log_error("Editor resource database: failed to initialize serializer while loading resource!\n");
			return Vadon::Model::ResourceHandle();
		}

		Vadon::Model::ResourceHandle loaded_resource_handle = resource_system.load_resource(*serializer_instance);
		if (loaded_resource_handle.is_valid() == false)
		{
			resource_system.log_error("Editor resource database: failed to load resource data!\n");
			return loaded_resource_handle;
		}

		if (serializer_instance->finalize() == false)
		{
			resource_system.log_error("Editor resource database: failed to finalize serializer after loading resource!\n");
		}

		return loaded_resource_handle;
	}

	Vadon::Core::FileInfo ResourceDatabase::get_file_resource_info(Vadon::Model::ResourceID resource_id) const
	{
		const Vadon::Core::FileDatabaseHandle asset_db = get_database(FileDatabaseType::ASSET_FILE);

		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();
		return file_system.get_file_info(asset_db, resource_id);
	}

	bool ResourceDatabase::load_resource_data(Vadon::Model::ResourceID resource_id, Vadon::Core::RawFileDataBuffer& file_data) const
	{
		const Vadon::Core::FileDatabaseHandle resource_db = get_database(FileDatabaseType::RESOURCE);

		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();
		return file_system.load_file(resource_db, resource_id, file_data);
	}

	bool ResourceDatabase::load_file_resource_data(Vadon::Model::ResourceID resource_id, Vadon::Core::RawFileDataBuffer& file_data) const
	{
		const Vadon::Core::FileDatabaseHandle asset_db = get_database(FileDatabaseType::ASSET_FILE);

		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();
		return file_system.load_file(asset_db, resource_id, file_data);
	}

	const ResourceDatabaseEntry* ResourceDatabase::find_resource_entry(Vadon::Model::ResourceID resource_id) const
	{
		auto info_it = m_resource_entry_lookup.find(resource_id);
		if (info_it == m_resource_entry_lookup.end())
		{
			return nullptr;
		}

		return &info_it->second;
	}

	Vadon::Model::ResourceID ResourceDatabase::import_resource(std::string_view path)
	{
		Vadon::Core::RawFileDataBuffer resource_file_buffer;
		const Vadon::Core::FileDatabaseHandle file_db = get_database(FileDatabaseType::RESOURCE);

		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();

		const Vadon::Core::FileID existing_file_id = file_system.find_file(file_db, path);
		if (existing_file_id.is_valid() == true)
		{
			// Resource already imported to DB
			return existing_file_id;
		}

		const std::string file_abs_path = file_system.get_absolute_path(file_db, path);

		if (file_system.load_file(file_abs_path, resource_file_buffer) == false)
		{
			Vadon::Core::Logger::log_error("Editor resource database: failed to load resource file!\n");
			return Vadon::Model::ResourceID();
		}

		// This is a "raw" editor file, we have to clear the labels from the keys
		// FIXME: find a better way, maybe using simdjson and iterators?
		if (sanitize_editor_resource_file(resource_file_buffer) == false)
		{
			Vadon::Core::Logger::log_error("Editor resource database: failed to process resource file data!\n");
			return Vadon::Model::ResourceID();
		}

		Vadon::Utilities::Serializer::Instance serializer_instance = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);

		// FIXME: support binary file serialization!
		// Solution: have file system create the appropriate serializer!
		if (serializer_instance->initialize() == false)
		{
			Vadon::Core::Logger::log_error("Editor resource database: failed to initialize serializer while loading resource!\n");
			return Vadon::Model::ResourceID();
		}

		Vadon::Model::ResourceInfo imported_resource_info;

		Vadon::Model::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Model::ResourceSystem>();
		if (resource_system.load_resource_info(*serializer_instance, imported_resource_info) == false)
		{
			Vadon::Core::Logger::log_error("Editor resource database: failed to load resource info!\n");
			return Vadon::Model::ResourceID();
		}

		if (serializer_instance->finalize() == false)
		{
			Vadon::Core::Logger::log_error("Editor resource database: failed to finalize serializer while loading resource!\n");
			return Vadon::Model::ResourceID();
		}

		const ResourceDatabaseEntry* resource_entry = find_resource_entry(imported_resource_info.id);
		if (resource_entry != nullptr)
		{
			// Resource already imported
			VADON_ASSERT(resource_entry->path == path, "Importing same resource from different paths!");
		}

		internal_import_resource(imported_resource_info, path);
		return imported_resource_info.id;
	}

	bool ResourceDatabase::import_project_resources()
	{
		// Import all resources in the project
		// FIXME: make use of a cache so we don't have to load every resource to get its ID
		// Will need to check whether something changed between the cache and the actual files
		const std::filesystem::path root_fs_path(m_project_manager.get_active_project().root_path);

		bool all_valid = true;

		for (const auto& directory_entry : std::filesystem::recursive_directory_iterator(root_fs_path))
		{
			if (directory_entry.is_regular_file() == false)
			{
				continue;
			}

			const ::Vadon::Foundation::ResourceFileInfo::Type current_asset_type = get_file_asset_type(directory_entry.path());
			if (current_asset_type != ::Vadon::Foundation::ResourceFileInfo::Type::NONE)
			{
				const std::string relative_path = std::filesystem::relative(directory_entry.path(), root_fs_path).generic_string();
				all_valid &= import_resource(relative_path).is_valid();
			}
		}

		return all_valid;
	}

	std::vector<Vadon::Model::ResourceID> ResourceDatabase::get_resource_list() const
	{
		std::vector<Vadon::Model::ResourceID> resource_list;

		for (const auto& entry_pair : m_resource_entry_lookup)
		{
			resource_list.push_back(entry_pair.first);
		}

		return resource_list;
	}

	bool ResourceDatabase::sanitize_editor_resource_file(Vadon::Core::RawFileDataBuffer& file_data)
	{
		return simdjson_sanitize_json_data(file_data);
	}

	void ResourceDatabase::internal_import_resource(const Vadon::Model::ResourceInfo& resource_info, std::string_view path)
	{
		const Vadon::Core::FileDatabaseHandle resource_file_db = get_database(FileDatabaseType::RESOURCE);

		Vadon::Core::FileInfo file_info;
		file_info.path = path;

		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();
		if (file_system.add_existing_file(resource_file_db, resource_info.id, file_info) == false)
		{
			// TODO: log error?
			VADON_ERROR("Cannot add resource file!");
			return;
		}

		if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Model::FileResource>(), resource_info.type_id) == true)
		{
			// Resource points to file, make sure we import that file as well
			if (import_asset_file(resource_info.id) == false)
			{
				// TODO: log error?
				VADON_ERROR("Cannot add file referenced by resource!");
				return;
			}
		}

		// Add to lookup
		// We may or may not have the resource data loaded, but we will need to cache the resource info
		ResourceDatabaseEntry resource_entry;
		resource_entry.base_info = resource_info;
		resource_entry.path = path;

		m_resource_entry_lookup.insert(std::make_pair(resource_info.id, resource_entry));
	}

	bool ResourceDatabase::import_asset_file(Vadon::Model::ResourceID file_id)
	{
		// NOTE: this assumes the resource file has already been registered!
		const Vadon::Core::FileDatabaseHandle resource_db = get_database(FileDatabaseType::RESOURCE);
		const Vadon::Core::FileDatabaseHandle asset_db = get_database(FileDatabaseType::ASSET_FILE);

		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();

		const Vadon::Core::FileInfo resource_file_info = file_system.get_file_info(resource_db, file_id);
		const std::filesystem::path resource_file_path = std::filesystem::path(resource_file_info.path).generic_string();

		VADON_ASSERT(resource_file_path.extension() == std::format(".{}", ::Vadon::Foundation::ResourceFileInfo::get_file_extension(::Vadon::Foundation::ResourceFileInfo::Type::IMPORTED_FILE)), "Invalid file type!");

		const std::filesystem::path imported_file_path = (resource_file_path.parent_path() / resource_file_path.stem()).generic_string();

		// Make sure the file actually exists at the designated path
		if (std::filesystem::exists(file_system.get_absolute_path(asset_db, imported_file_path.generic_string())) == false)
		{
			// TODO: log error!
			return false;
		}

		Vadon::Core::FileInfo imported_file_info;
		imported_file_info.path = imported_file_path.generic_string();

		if (file_system.add_existing_file(asset_db, file_id, imported_file_info) == false)
		{
			// TODO: log error!
			return false;
		}

		return true;
	}
}