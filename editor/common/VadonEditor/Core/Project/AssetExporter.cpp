#include <VadonEditor/Core/Project/AssetExporter.hpp>

#include <VadonEditor/Core/AssetServer.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/Resource/Database.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/Model/Resource/ResourceSystem.hpp>
#include <Vadon/Model/Resource/File.hpp>

#include <Vadon/Utilities/Debugging/Assert.hpp>
#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <filesystem>

namespace
{
	// FIXME: this is a very hacky solution, should instead use "manifests" to map between resource IDs and files
	Vadon::Model::ResourceID decode_resource_id_from_file(const std::filesystem::path& file_path)
	{
		// Decode resource ID from file name
		const std::string file_stem = file_path.stem().generic_string();
		Vadon::Model::ResourceID resource_id;
		VADON_ASSERT(file_stem.length() == ::Vadon::Foundation::UUID::c_uuid_width * 2, "Invalid file name!");

		return Vadon::Utilities::uuid_from_hex_string(file_stem);
	}

	bool is_type_file_resource(Vadon::Utilities::TypeID type_id)
	{
		return Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Model::FileResource>(), type_id);
	}

	bool is_file_metadata_equal(const Vadon::Core::FileMetadata& lhs, const Vadon::Core::FileMetadata& rhs)
	{
		return (lhs.last_write_time == rhs.last_write_time) && (lhs.size == rhs.size);
	}

	std::string get_project_file_path(std::string_view root_path)
	{
		return (std::filesystem::path(root_path) / Vadon::Core::Project::c_project_file_name).generic_string();;
	}
}

namespace VadonEditor::Core
{
	void AssetExporter::Database::update_cache(const ::Vadon::Foundation::UUID& file_id, const Vadon::Core::FileInfo& file_info)
	{
		VADON_ASSERT(file_info.is_valid() == true, "Invalid file info!");

		auto cache_it = cache.find(file_id);
		if (cache_it == cache.end())
		{
			CacheEntry new_cache_entry;
			cache_it = cache.insert(std::make_pair(file_id, new_cache_entry)).first;
		}

		// TODO: add other metadata!
		CacheEntry& cache_entry = cache_it->second;
		cache_entry.file_metadata = file_info.metadata;
	}

	void AssetExporter::Database::remove_from_cache(const::Vadon::Foundation::UUID& file_id)
	{
		auto cache_it = cache.find(file_id);
		if (cache_it != cache.end())
		{
			cache.erase(cache_it);
		}
	}

	bool AssetExporter::Database::is_file_up_to_date(const::Vadon::Foundation::UUID& file_id, const Vadon::Core::FileInfo& current_file_info) const
	{
		auto cache_it = cache.find(file_id);
		if (cache_it == cache.end())
		{
			// File not in cache, so it's definitely not up-to-date
			return false;
		}

		const CacheEntry& cache_entry = cache_it->second;
		return is_file_metadata_equal(cache_entry.file_metadata, current_file_info.metadata);
	}

	AssetExporter::AssetExporter(AssetServer& asset_server)
		: m_asset_server(asset_server)
	{

	}

	bool AssetExporter::initialize()
	{
		// TODO: any project-agnostic initialization?
		return true;
	}

	bool AssetExporter::initialize_output(std::string_view output_path)
	{
		if (output_path.empty() == true)
		{
			Vadon::Core::Logger::log_error("Failed to start asset exporter, output path is empty!\n");
			return false;
		}

		m_output_root = output_path;

		Vadon::Core::EngineCoreInterface& engine_core = m_asset_server.get_engine_core();
		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

		{
			Vadon::Core::FileDatabaseInfo resource_db_info;
			resource_db_info.root_path = (std::filesystem::path(output_path) / "resources").generic_string();
			resource_db_info.type = Vadon::Core::FileDatabaseType::FILESYSTEM;

			m_resource_database.file_db_handle = file_system.create_database(resource_db_info);
		}

		{
			Vadon::Core::FileDatabaseInfo asset_db_info;
			asset_db_info.root_path = (std::filesystem::path(output_path) / "assets").generic_string();
			asset_db_info.type = Vadon::Core::FileDatabaseType::FILESYSTEM;

			m_asset_database.file_db_handle = file_system.create_database(asset_db_info);
		}

		return true;
	}

	void AssetExporter::update()
	{
		// First refresh the source DB
		VadonEditor::Model::ResourceDatabase& resource_database = m_asset_server.get_resource_database();
		resource_database.refresh();

		// Clear any exported files that have become stale
		clear_stale_files();

		// Get all the current entries in the source DB
		Vadon::Core::EngineCoreInterface& engine_core = m_asset_server.get_engine_core();

		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();	

		const Vadon::Core::FileDatabaseHandle src_resource_db_handle = resource_database.get_database(VadonEditor::Model::ResourceDatabase::FileDatabaseType::RESOURCE);
		const Vadon::Core::FileDatabaseHandle src_asset_db_handle = resource_database.get_database(VadonEditor::Model::ResourceDatabase::FileDatabaseType::ASSET_FILE);

		const std::vector<Vadon::Model::ResourceID> resource_list = resource_database.get_resource_list();

		for (const Vadon::Model::ResourceID& current_resource_id : resource_list)
		{
			const Vadon::Core::FileInfo current_file_info = file_system.get_file_info(m_resource_database.file_db_handle, current_resource_id);
			if ((current_file_info.is_valid() == true) && (current_file_info.metadata.exists == true))
			{
				// Resource file already exists, check cache to see if it needs to be updated
				const Vadon::Core::FileInfo src_resource_file_info = file_system.get_file_info(src_resource_db_handle, current_resource_id);
				if (m_resource_database.is_file_up_to_date(current_resource_id, src_resource_file_info) == false)
				{
					if (export_resource_file(current_resource_id) == false)
					{
						// TODO: log error
					}
				}

				const VadonEditor::Model::ResourceDatabaseEntry* resource_db_entry = resource_database.find_resource_entry(current_resource_id);
				if (is_type_file_resource(resource_db_entry->base_info.type_id) == true)
				{
					// Also check the asset file
					const Vadon::Core::FileInfo src_asset_file_info = file_system.get_file_info(src_asset_db_handle, current_resource_id);
					if (m_asset_database.is_file_up_to_date(current_resource_id, src_asset_file_info) == false)
					{
						if (export_asset_file(current_resource_id) == false)
						{
							// TODO: log error
						}
					}
				}
			}
			else
			{
				// Newly added resource, export it
				if (export_file(current_resource_id) == false)
				{
					// TODO: error message!
				}
			}
		}

		// Check project file
		{
			const Vadon::Core::Project& active_project = m_asset_server.get_project_manager().get_active_project();
			const std::string source_project_file_path = get_project_file_path(active_project.root_path);

			const Vadon::Core::FileMetadata project_file_metadata = file_system.get_file_metadata(source_project_file_path);
			if (project_file_metadata.exists == true)
			{
				if (is_file_metadata_equal(m_project_file_metadata, project_file_metadata) == false)
				{
					// Metadata mismatch, re-export
					export_project_file();
				}
			}
			else
			{
				// File is missing, re-export
				export_project_file();
			}
		}
	}

	bool AssetExporter::export_file(const ::Vadon::Foundation::UUID& resource_id)
	{
		// Export the resource file itself
		if (export_resource_file(resource_id) == false)
		{
			return false;
		}

		// Check whether we need to export additional content
		const VadonEditor::Model::ResourceDatabase& resource_database = m_asset_server.get_resource_database();
		const VadonEditor::Model::ResourceDatabaseEntry* resource_db_entry = resource_database.find_resource_entry(resource_id);
		if (is_type_file_resource(resource_db_entry->base_info.type_id) == true)
		{
			// Resource points to a file, so we need to export that as well
			if (export_asset_file(resource_id) == false)
			{
				return false;
			}
		}

		return true;
	}

	bool AssetExporter::export_resource_file(const ::Vadon::Foundation::UUID& resource_id)
	{
		m_source_file_data_buffer.clear();

		const VadonEditor::Model::ResourceDatabase& resource_database = m_asset_server.get_resource_database();
		if (resource_database.load_resource_data(resource_id, m_source_file_data_buffer) == false)
		{
			Vadon::Core::Logger::log_error("Asset server exporter: failed to load resource file contents!\n");
			return false;
		}

		if (VadonEditor::Model::ResourceDatabase::sanitize_editor_resource_file(m_source_file_data_buffer) == false)
		{
			Vadon::Core::Logger::log_error("Asset server exporter: failed to sanitize resource file contents!\n");
			return false;
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_asset_server.get_engine_core();
		Vadon::Model::ResourceSystem& engine_resource_system = engine_core.get_system<Vadon::Model::ResourceSystem>();

		Vadon::Utilities::VariantDictionary resource_raw_data;
		{
			Vadon::Utilities::Serializer::Instance json_serializer = Vadon::Utilities::Serializer::create_serializer(m_source_file_data_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);

			if (json_serializer->initialize() == false)
			{
				Vadon::Core::Logger::log_error("Asset server exporter: failed to initialize serializer while loading resource!\n");
				return false;
			}

			if (engine_resource_system.load_resource_raw_data(*json_serializer, resource_raw_data) == false)
			{
				// FIXME: allow graceful exit so the asset server doesn't crash?
				Vadon::Core::Logger::log_error("Asset server exporter: failed to load resource data!\n");
				return false;
			}

			if (json_serializer->finalize() == false)
			{
				Vadon::Core::Logger::log_error("Asset server exporter: failed to finalize import serializer!\n");
				return false;
			}
		}

		m_exported_file_data_buffer.clear();
		{
			Vadon::Utilities::Serializer::Instance binary_serializer = Vadon::Utilities::Serializer::create_serializer(m_exported_file_data_buffer, Vadon::Utilities::Serializer::Type::BINARY, Vadon::Utilities::Serializer::Mode::WRITE);
			if (binary_serializer->initialize() == false)
			{
				Vadon::Core::Logger::log_error("Asset server exporter: failed to initialize export serializer!\n");
				return false;
			}
			if (engine_resource_system.save_resource_raw_data(*binary_serializer, resource_raw_data) == false)
			{
				Vadon::Core::Logger::log_error("Asset server exporter: failed to serialize resource!\n");
				binary_serializer->finalize();
				return false;
			}
			if (binary_serializer->finalize() == false)
			{
				Vadon::Core::Logger::log_error("Asset server exporter: failed to finalize export serializer!\n");
				return false;
			}
		}

		Vadon::Core::FileInfo file_info;
		file_info.offset = 0; // TODO: allow for files to be "packaged" into one file
		file_info.size = static_cast<int>(m_exported_file_data_buffer.size());

		// Use hex representation of resource UUID as the file name
		file_info.path += Vadon::Utilities::uuid_to_hex_string(resource_id) + ".vdbin";

		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();
		if (file_system.get_file_info(m_resource_database.file_db_handle, resource_id).is_valid() == false)
		{
			// Add to asset library via the file DB
			file_system.add_existing_file(m_resource_database.file_db_handle, resource_id, file_info);
		}

		if (file_system.save_file(m_resource_database.file_db_handle, resource_id, m_exported_file_data_buffer) == false)
		{
			Vadon::Core::Logger::log_error("Asset server exporter: failed to save resource to file!\n");
			return false;
		}

		std::string resource_id_string = Vadon::Utilities::uuid_to_string(resource_id).string;

		const Vadon::Core::FileDatabaseHandle src_resource_db_handle = resource_database.get_database(VadonEditor::Model::ResourceDatabase::FileDatabaseType::RESOURCE);
		const Vadon::Core::FileInfo src_resource_file_info = file_system.get_file_info(src_resource_db_handle, resource_id);
		if (src_resource_file_info.is_valid() == true)
		{
			resource_id_string = src_resource_file_info.path;

			// Update cache with metadata of the version of the file we just exporrted
			m_resource_database.update_cache(resource_id, src_resource_file_info);
		}

		Vadon::Core::Logger::log_message(std::format("Asset server exporter: exported resource {} to {}\n", resource_id_string, file_info.path));
		return true;
	}

	bool AssetExporter::export_asset_file(const ::Vadon::Foundation::UUID& asset_file_id)
	{
		// Add to the export database
		Vadon::Core::FileInfo asset_file_info;
		asset_file_info.offset = 0; // TODO: allow for files to be "packaged" into one file
		asset_file_info.size = 0; // FIXME: get the file size!

		// Same path as resource, but different DB root
		asset_file_info.path = Vadon::Utilities::uuid_to_hex_string(asset_file_id) + ".vdbin";

		Vadon::Core::EngineCoreInterface& engine_core = m_asset_server.get_engine_core();
		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

		if (file_system.get_file_info(m_asset_database.file_db_handle, asset_file_id).is_valid() == false)
		{
			// Add to asset library via the file DB
			file_system.add_existing_file(m_asset_database.file_db_handle, asset_file_id, asset_file_info);
		}

		// Copy from asset library to export destination
		const VadonEditor::Model::ResourceDatabase& resource_database = m_asset_server.get_resource_database();
		const Vadon::Core::FileDatabaseHandle src_asset_db_handle = resource_database.get_database(VadonEditor::Model::ResourceDatabase::FileDatabaseType::ASSET_FILE);
		if (file_system.copy_file(src_asset_db_handle, asset_file_id, m_asset_database.file_db_handle, asset_file_id) == false)
		{
			Vadon::Core::Logger::log_error("Asset Server exporter: failed to copy asset file!\n");
			return false;
		}

		std::string asset_id_string = Vadon::Utilities::uuid_to_string(asset_file_id).string;

		const Vadon::Core::FileInfo src_asset_file_info = file_system.get_file_info(src_asset_db_handle, asset_file_id);
		if (src_asset_file_info.is_valid() == true)
		{
			asset_id_string = src_asset_file_info.path;

			// Update cache with metadata of the version of the file we just copied
			m_asset_database.update_cache(asset_file_id, src_asset_file_info);
		}

		Vadon::Core::Logger::log_message(std::format("Asset server exporter: exported asset {} to {}\n", asset_id_string, asset_file_info.path));
		return true;
	}

	bool AssetExporter::export_project_file()
	{
		Vadon::Core::RawFileDataBuffer project_file_data;
		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(project_file_data, Vadon::Utilities::Serializer::Type::BINARY, Vadon::Utilities::Serializer::Mode::WRITE);
		if (serializer->initialize() == false)
		{
			Vadon::Core::Logger::log_error("Asset server exporter: failed to initialize project serializer!\n");
			return false;
		}

		const Vadon::Core::Project& active_project_info = m_asset_server.get_project_manager().get_active_project();

		// Load the project info from file 
		// The one in the manager might be different, though the root path will be the same
		Vadon::Core::Project loaded_project_info;
		if (ProjectManager::load_project_data(m_asset_server.get_engine_core(), active_project_info.root_path, loaded_project_info) == false)
		{
			Vadon::Core::Logger::log_error("Asset server: failed to load source project file!\n");
			return false;
		}

		if (Vadon::Core::Project::serialize_project_data(*serializer, loaded_project_info) == false)
		{
			Vadon::Core::Logger::log_error("Asset server: error serializing project data!\n");
			return false;
		}

		if (serializer->finalize() == false)
		{
			Vadon::Core::Logger::log_error("Asset server exporter: failed to finalize project serializer!\n");
			return false;
		}

		const std::string file_path = (std::filesystem::path(m_output_root) / Vadon::Core::Project::c_project_file_name).generic_string();

		Vadon::Core::EngineCoreInterface& engine_core = m_asset_server.get_engine_core();
		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

		if (file_system.save_file(file_path, project_file_data) == false)
		{
			Vadon::Core::Logger::log_error("Asset server: error exporting project file!\n");
			return false;
		}

		// Store metadata to check that file is up-to-date
		const std::string source_project_file_path = get_project_file_path(active_project_info.root_path);
		const Vadon::Core::FileMetadata src_project_file_metadata = file_system.get_file_metadata(source_project_file_path);
		if (src_project_file_metadata.exists == true)
		{
			m_project_file_metadata = src_project_file_metadata;
		}
		else
		{
			Vadon::Core::Logger::log_error("Asset server: failed to get source project file metadata!\n");
		}

		Vadon::Core::Logger::log_message(std::format("Asset Server exporter: exported project file to {}!\n", file_path));
		return true;
	}

	bool AssetExporter::export_all_project_data()
	{
		const VadonEditor::Model::ResourceDatabase& resource_database = m_asset_server.get_resource_database();
		const std::vector<Vadon::Model::ResourceID> resource_list = resource_database.get_resource_list();

		for (const Vadon::Model::ResourceID& current_resource_id : resource_list)
		{
			if (export_file(current_resource_id) == false)
			{
				// TODO: error message!
			}
		}

		if (export_project_file() == false)
		{
			// TODO: error message
		}

		// Check for any files which should be removed
		clear_stale_files();

		Vadon::Core::Logger::log_message(std::format("Asset Server exporter: exported project data to {}!\n", m_output_root));
		return true;
	}

	void AssetExporter::clear_stale_files()
	{
		const VadonEditor::Model::ResourceDatabase& resource_database = m_asset_server.get_resource_database();
		const Vadon::Core::FileDatabaseHandle src_resource_db_handle = resource_database.get_database(VadonEditor::Model::ResourceDatabase::FileDatabaseType::RESOURCE);
		const Vadon::Core::FileDatabaseHandle src_asset_db_handle = resource_database.get_database(VadonEditor::Model::ResourceDatabase::FileDatabaseType::ASSET_FILE);

		Vadon::Core::EngineCoreInterface& engine_core = m_asset_server.get_engine_core();
		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

		auto remove_file_func = +[](std::string_view path)
			{
				std::error_code fs_error;
				if (std::filesystem::remove(path, fs_error) == true)
				{
					Vadon::Core::Logger::log_message(std::format("Asset server exporter: removed stale file {}\n", path));
				}
				else
				{
					Vadon::Core::Logger::log_error(std::format("Asset server exporter: failed to remove stale file {} (error: {})\n", path, fs_error.message()));
				}
			};

		const std::vector<Vadon::Core::FileID> all_files = file_system.get_all_files(m_resource_database.file_db_handle);
		for (const Vadon::Core::FileID& current_file_id : all_files)
		{
			const Vadon::Core::FileInfo current_file_info = file_system.get_file_info(m_resource_database.file_db_handle, current_file_id);
			if (current_file_info.is_valid() == false)
			{
				// Invalid file, error!
				continue;
			}

			// Check if the source database still contains this file
			if (file_system.does_file_exist(src_resource_db_handle, current_file_id) == true)
			{
				// File still present, check next file
				continue;
			}

			// Source file no longer exists, remove it from database and the file system
			file_system.remove_file(m_resource_database.file_db_handle, current_file_id);

			// Remove cache entry
			m_resource_database.remove_from_cache(current_file_id);

			// Check if the exported file actually exists in the file system
			if (current_file_info.metadata.exists == true)
			{
				// Remove the file
				remove_file_func(current_file_info.path);
			}

			// Check if the file also connects to an asset file
			if (file_system.does_file_exist(m_asset_database.file_db_handle, current_file_id) == true)
			{
				// Also remove from asset file DB
				const Vadon::Core::FileInfo asset_file_info = file_system.get_file_info(m_asset_database.file_db_handle, current_file_id);
				if (asset_file_info.is_valid() == true)
				{
					file_system.remove_file(m_asset_database.file_db_handle, current_file_id);

					// Remove cache entry
					m_asset_database.remove_from_cache(current_file_id);

					if (asset_file_info.metadata.exists == true)
					{
						remove_file_func(asset_file_info.path);
					}
				}
			}
		}
	}
}