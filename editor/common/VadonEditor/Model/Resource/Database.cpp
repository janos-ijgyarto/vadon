#include <VadonEditor/Model/Resource/Database.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <Vadon/Model/Resource/File.hpp>
#include <Vadon/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <filesystem>

namespace VadonEditor::Model
{

	ResourceDatabase::ResourceDatabase(Core::Editor& editor)
		: m_editor(editor)
	{

	}

	bool ResourceDatabase::initialize()
	{
		Core::ProjectManager& project_manager = m_editor.get_project_manager();

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

		// Create the databases for resources and assets
		{
			Vadon::Core::FileDatabaseInfo resource_db_info;
			resource_db_info.root_path = project_manager.get_active_project().root_path;
			resource_db_info.type = Vadon::Core::FileDatabaseType::FILESYSTEM;

			m_file_databases[static_cast<size_t>(FileDatabaseType::RESOURCE)] = file_system.create_database(resource_db_info);
		}

		{
			Vadon::Core::FileDatabaseInfo asset_db_info;
			asset_db_info.root_path = project_manager.get_active_project().root_path;
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

		Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
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

		Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
		Vadon::Core::RawFileDataBuffer resource_file_buffer;

		if (file_system.load_file(resource_file_db, resource_id, resource_file_buffer) == false)
		{
			resource_system.log_error("Editor resource database: failed to load resource file!\n");
			return Vadon::Model::ResourceHandle();
		}

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

		Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
		return file_system.get_file_info(asset_db, resource_id);
	}

	bool ResourceDatabase::load_file_resource_data(Vadon::Model::ResourceSystem& /*resource_system*/, Vadon::Model::ResourceID resource_id, Vadon::Core::RawFileDataBuffer& file_data)
	{
		const Vadon::Core::FileDatabaseHandle asset_db = get_database(FileDatabaseType::ASSET_FILE);

		Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
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

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

		const std::string file_abs_path = file_system.get_absolute_path(file_db, path);

		if (file_system.load_file(file_abs_path, resource_file_buffer) == false)
		{
			Vadon::Core::Logger::log_error("Editor resource database: failed to load resource file!\n");
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

		Vadon::Model::ResourceSystem& resource_system = engine_core.get_system<Vadon::Model::ResourceSystem>();
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

	void ResourceDatabase::internal_import_resource(const Vadon::Model::ResourceInfo& resource_info, std::string_view path)
	{
		const Vadon::Core::FileDatabaseHandle resource_file_db = get_database(FileDatabaseType::RESOURCE);

		Vadon::Core::FileInfo file_info;
		file_info.path = path;

		Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
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

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

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