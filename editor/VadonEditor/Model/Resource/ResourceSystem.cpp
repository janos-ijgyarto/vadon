#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/Resource/Resource.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/Scene/Resource/File.hpp>
#include <Vadon/Scene/Resource/ResourceSystem.hpp>
#include <Vadon/Scene/Scene.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <filesystem>

namespace
{
	VadonEditor::Core::AssetType get_file_asset_type(const std::filesystem::path& path)
	{
		const std::string extension = path.extension().generic_string();
		if (extension == VadonEditor::Core::AssetInfo::get_file_extension(VadonEditor::Core::AssetType::SCENE))
		{
			return VadonEditor::Core::AssetType::SCENE;
		}
		else if (extension == VadonEditor::Core::AssetInfo::get_file_extension(VadonEditor::Core::AssetType::RESOURCE))
		{
			return VadonEditor::Core::AssetType::RESOURCE;
		}
		else if (extension == VadonEditor::Core::AssetInfo::get_file_extension(VadonEditor::Core::AssetType::IMPORTED_FILE))
		{
			return VadonEditor::Core::AssetType::IMPORTED_FILE;
		}

		return VadonEditor::Core::AssetType::NONE;
	}
}

namespace VadonEditor::Model
{
	ResourceSystem::EditorResourceDatabaseImpl::EditorResourceDatabaseImpl(Core::Editor& editor)
		: m_editor(editor)
	{
	}

	bool ResourceSystem::EditorResourceDatabaseImpl::initialize()
	{
		Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

		// Create the databases for resources and assets
		{
			Vadon::Core::FileDatabaseInfo resource_db_info;
			resource_db_info.root_path = project_manager.get_active_project().info.root_path;
			resource_db_info.type = Vadon::Core::FileDatabaseType::FILESYSTEM;

			m_file_databases[static_cast<size_t>(FileDatabaseType::RESOURCE)] = file_system.create_database(resource_db_info);
		}

		{
			Vadon::Core::FileDatabaseInfo asset_db_info;
			asset_db_info.root_path = project_manager.get_active_project().info.root_path;
			asset_db_info.type = Vadon::Core::FileDatabaseType::FILESYSTEM;

			m_file_databases[static_cast<size_t>(FileDatabaseType::ASSET_FILE)] = file_system.create_database(asset_db_info);
		}

		return true;
	}

	bool ResourceSystem::EditorResourceDatabaseImpl::save_resource(Vadon::Scene::ResourceSystem& resource_system, Vadon::Scene::ResourceHandle resource_handle)
	{
		const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);

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

	Vadon::Scene::ResourceHandle ResourceSystem::EditorResourceDatabaseImpl::load_resource(Vadon::Scene::ResourceSystem& resource_system, ResourceID resource_id)
	{
		const Vadon::Core::FileDatabaseHandle resource_file_db = get_database(FileDatabaseType::RESOURCE);

		Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
		Vadon::Core::RawFileDataBuffer resource_file_buffer;

		if (file_system.load_file(resource_file_db, resource_id, resource_file_buffer) == false)
		{
			resource_system.log_error("Editor resource database: failed to load resource file!\n");
			return Vadon::Scene::ResourceHandle();
		}

		// FIXME: support binary file serialization!
		// Solution: have file system create the appropriate serializer!
		Vadon::Utilities::Serializer::Instance serializer_instance = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);

		if (serializer_instance->initialize() == false)
		{
			resource_system.log_error("Editor resource database: failed to initialize serializer while loading resource!\n");
			return Vadon::Scene::ResourceHandle();
		}

		Vadon::Scene::ResourceHandle loaded_resource_handle = resource_system.load_resource(*serializer_instance);
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

	Vadon::Core::FileInfo ResourceSystem::EditorResourceDatabaseImpl::get_file_resource_info(Vadon::Scene::ResourceID resource_id) const
	{
		const Vadon::Core::FileDatabaseHandle asset_db = get_database(FileDatabaseType::ASSET_FILE);

		Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
		return file_system.get_file_info(asset_db, resource_id);
	}

	bool ResourceSystem::EditorResourceDatabaseImpl::load_file_resource_data(Vadon::Scene::ResourceSystem& /*resource_system*/, ResourceID resource_id, Vadon::Core::RawFileDataBuffer& file_data)
	{
		const Vadon::Core::FileDatabaseHandle asset_db = get_database(FileDatabaseType::ASSET_FILE);

		Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
		return file_system.load_file(asset_db, resource_id, file_data);
	}

	const ResourceInfo* ResourceSystem::EditorResourceDatabaseImpl::find_resource_info(ResourceID resource_id) const
	{
		auto info_it = m_resource_info_lookup.find(resource_id);
		if (info_it == m_resource_info_lookup.end())
		{
			return nullptr;
		}

		return &info_it->second;
	}

	bool ResourceSystem::EditorResourceDatabaseImpl::save_resource_as(ResourceID resource_id, std::string_view path)
	{
		if (find_resource_info(resource_id) != nullptr)
		{
			// Resource has already been saved to a file!
			return false;
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		const Vadon::Scene::ResourceHandle resource_handle = resource_system.find_resource(resource_id);
		if (resource_handle.is_valid() == false)
		{
			Vadon::Core::Logger::log_error("Editor resource database: resource is not loaded!\n");
			return false;
		}

		const Vadon::Core::FileDatabaseHandle resource_file_db = get_database(FileDatabaseType::RESOURCE);

		Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
		if (file_system.does_file_exist(resource_file_db, resource_id) == true)
		{
			Vadon::Core::Logger::log_error("Editor resource database: resource already has associated file!\n");
			return false;
		}

		// Get info and import (adds resource to file database)
		const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);
		internal_import_resource(resource_info, file_system.get_relative_path(resource_file_db, path));

		return save_resource(resource_system, resource_handle);
	}

	ResourceID ResourceSystem::EditorResourceDatabaseImpl::import_resource(std::string_view path)
	{
		Vadon::Core::RawFileDataBuffer resource_file_buffer;
		const Vadon::Core::FileDatabaseHandle file_db = get_database(FileDatabaseType::RESOURCE);

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

		const std::string file_abs_path = file_system.get_absolute_path(file_db, path);

		if (file_system.load_file(file_abs_path, resource_file_buffer) == false)
		{
			Vadon::Core::Logger::log_error("Editor resource database: failed to load resource file!\n");
			return ResourceID();
		}

		Vadon::Utilities::Serializer::Instance serializer_instance = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);

		// FIXME: support binary file serialization!
		// Solution: have file system create the appropriate serializer!
		if (serializer_instance->initialize() == false)
		{
			Vadon::Core::Logger::log_error("Editor resource database: failed to initialize serializer while loading resource!\n");
			return ResourceID();
		}

		Vadon::Scene::ResourceInfo imported_resource_info;

		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
		if (resource_system.load_resource_info(*serializer_instance, imported_resource_info) == false)
		{
			Vadon::Core::Logger::log_error("Editor resource database: failed to loading resource!\n");
			return ResourceID();
		}

		if (serializer_instance->finalize() == false)
		{
			Vadon::Core::Logger::log_error("Editor resource database: failed to finalize serializer while loading resource!\n");
			return ResourceID();
		}
		
		const ResourceInfo* resource_info = find_resource_info(imported_resource_info.id);
		if (resource_info != nullptr)
		{
			// Resource already imported
			VADON_ASSERT(resource_info->path == path, "Importing same resource from different paths!");
		}

		internal_import_resource(imported_resource_info, path);		
		return imported_resource_info.id;
	}

	std::vector<ResourceInfo> ResourceSystem::EditorResourceDatabaseImpl::get_resource_list(Vadon::Utilities::TypeID resource_type) const
	{
		std::vector<ResourceInfo> result;

		for (const auto& current_resource_pair : m_resource_info_lookup)
		{
			const Vadon::Scene::ResourceInfo& current_resource_info = current_resource_pair.second.info;
			if(Vadon::Utilities::TypeRegistry::is_base_of(resource_type, current_resource_info.type_id))
			{
				result.push_back(current_resource_pair.second);
			}
		}

		return result;
	}

	void ResourceSystem::EditorResourceDatabaseImpl::internal_import_resource(const Vadon::Scene::ResourceInfo& resource_info, std::string_view path)
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

		if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Scene::FileResource>(), resource_info.type_id) == true)
		{
			// Resource points to file, make sure we import that file as well
			if (import_asset_file(resource_info.id) == false)
			{
				// TODO: log error?
				VADON_ERROR("Cannot add file referenced by resource!");
				return;
			}
		}
		else
		{
			const bool is_scene = Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Scene::Scene>(), resource_info.type_id);

			Core::AssetLibrary& asset_library = m_editor.get_system<Core::ProjectManager>().get_asset_library();

			Core::AssetInfo resource_asset_info;
			resource_asset_info.name = std::filesystem::path(path).stem().generic_string();
			resource_asset_info.type = is_scene ? Core::AssetType::SCENE : Core::AssetType::RESOURCE;
			resource_asset_info.file_id = resource_info.id;

			asset_library.create_node(resource_asset_info, path);
		}

		// Add to lookup
		// We may or may not have the resource data loaded, but we will need to cache the resource info
		ResourceInfo editor_resource_info;
		editor_resource_info.info = resource_info;
		editor_resource_info.path = path;

		m_resource_info_lookup.insert(std::make_pair(resource_info.id, editor_resource_info));
	}

	bool ResourceSystem::EditorResourceDatabaseImpl::import_asset_file(ResourceID file_id)
	{
		// NOTE: this assumes the resource file has already been registered!
		const Vadon::Core::FileDatabaseHandle resource_db = get_database(FileDatabaseType::RESOURCE);
		const Vadon::Core::FileDatabaseHandle asset_db = get_database(FileDatabaseType::ASSET_FILE);

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

		const Vadon::Core::FileInfo resource_file_info = file_system.get_file_info(resource_db, file_id);
		const std::filesystem::path resource_file_path = std::filesystem::path(resource_file_info.path).generic_string();

		VADON_ASSERT(resource_file_path.extension() == Core::AssetInfo::get_file_extension(Core::AssetType::IMPORTED_FILE), "Invalid file type!");

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

		// Add to asset library
		Core::AssetLibrary& asset_library = m_editor.get_system<Core::ProjectManager>().get_asset_library();

		Core::AssetInfo resource_asset_info;
		resource_asset_info.name = imported_file_path.stem().generic_string();
		resource_asset_info.type = Core::AssetType::IMPORTED_FILE;
		resource_asset_info.file_id = file_id;

		asset_library.create_node(resource_asset_info, imported_file_info.path);

		return true;
	}

	Resource* ResourceSystem::create_resource(Vadon::Utilities::TypeID resource_type)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		Vadon::Scene::ResourceHandle new_resource_handle = resource_system.create_resource(resource_type);
		if (new_resource_handle.is_valid() == false)
		{
			Vadon::Core::Logger::log_error("Editor resource system: failed to create resource!\n");
			return nullptr;
		}

		return get_resource(new_resource_handle);
	}

	Resource* ResourceSystem::get_resource(Vadon::Scene::ResourceID resource_id)
	{
		auto resource_it = m_resource_lookup.find(resource_id);
		if (resource_it == m_resource_lookup.end())
		{
			// Resource not yet registered, so we create it
			const EditorResourceID editor_resource_id = m_resource_id_counter++;
			resource_it = m_resource_lookup.emplace(resource_id, Resource(m_editor, resource_id, editor_resource_id)).first;

			// Check whether the resource was already loaded
			Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
			Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
			const Vadon::Scene::ResourceHandle resource_handle = resource_system.find_resource(resource_id);
			if (resource_handle.is_valid() == true)
			{
				// Resource already loaded
				resource_it->second.internal_load(resource_handle);
			}
		}

		return &resource_it->second;
	}

	Resource* ResourceSystem::get_resource(Vadon::Scene::ResourceHandle resource_handle)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		return get_resource(resource_system.get_resource_info(resource_handle).id);
	}

	void ResourceSystem::remove_resource(Resource* resource)
	{
		// TODO: use some kind of reference tracking to make sure we unset all references to this resource!

		// Remove from owner
		if (resource->is_embedded() == true)
		{
			for (size_t embedded_resource_index = 0; embedded_resource_index < resource->m_owner->m_embedded_resources.size(); ++embedded_resource_index)
			{
				if (resource->m_owner->m_embedded_resources[embedded_resource_index] == resource)
				{
					resource->m_owner->m_embedded_resources.erase(resource->m_owner->m_embedded_resources.begin() + embedded_resource_index);
					break;
				}
			}
		}

		// Remove embedded resources
		for (Resource* embedded_resource : resource->m_embedded_resources)
		{
			// Invalidate the handles of embedded resources (they will be recursively cleaned up by the engine resource system)
			embedded_resource->m_owner = nullptr;
			embedded_resource->m_handle.invalidate();

			// Remove the editor object
			remove_resource(embedded_resource);
		}

		// Remove the engine resource as well (unless we are already being cleaned up by owner)
		if (resource->m_handle.is_valid() == true)
		{
			Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
			Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

			if (resource->is_embedded() == true)
			{
				// Need to remove as an embedded resource
				resource_system.remove_embedded_resource(resource->m_owner->m_handle, resource->m_handle);
			}
			else
			{
				resource_system.remove_resource(resource->m_handle);
			}
		}
		
		m_resource_lookup.erase(resource->get_id());
	}

	void ResourceSystem::register_edit_callback(EditCallback callback)
	{
		m_edit_callbacks.push_back(callback);
	}

	void ResourceSystem::resource_edited(const Resource& resource)
	{
		for (const EditCallback& current_callback : m_edit_callbacks)
		{
			current_callback(resource.get_id());
		}
	}

	ResourceSystem::ResourceSystem(Core::Editor& editor)
		: m_editor(editor)
		, m_resource_id_counter(0)
		, m_database(editor)
	{

	}

	bool ResourceSystem::initialize()
	{
		// Register the resource DB
		m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>().register_database(m_database);
		return true;
	}

	bool ResourceSystem::load_project_resources()
	{
		// Initialize database
		if (m_database.initialize() == false)
		{
			return false;
		}

		// Import all resources in the project
		// FIXME: make use of a cache so we don't have to load every resource to get its ID
		// Will need to check whether something changed between the cache and the actual files
		Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();
		const std::filesystem::path root_fs_path(project_manager.get_active_project().info.root_path);

		bool all_valid = true;

		for (const auto& directory_entry : std::filesystem::recursive_directory_iterator(root_fs_path))
		{
			if (directory_entry.is_regular_file() == false)
			{
				continue;
			}

			const Core::AssetType current_asset_type = get_file_asset_type(directory_entry.path());
			if (current_asset_type != Core::AssetType::NONE)
			{
				const std::string relative_path = std::filesystem::relative(directory_entry.path(), root_fs_path).generic_string();
				all_valid &= m_database.import_resource(relative_path).is_valid();
			}
		}

		return all_valid;
	}

	bool ResourceSystem::export_project_resources(std::string_view output_path)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

		// Create the export databases
		// FIXME: generalize this by iterating over all the asset types!
		Vadon::Core::FileDatabaseHandle resource_db_handle;
		Vadon::Core::FileDatabaseHandle asset_db_handle;
		
		{
			Vadon::Core::FileDatabaseInfo resource_db_info;
			resource_db_info.root_path = (std::filesystem::path(output_path) / "resources").generic_string();
			resource_db_info.type = Vadon::Core::FileDatabaseType::FILESYSTEM;

			resource_db_handle = file_system.create_database(resource_db_info);
		}

		{
			Vadon::Core::FileDatabaseInfo asset_db_info;
			asset_db_info.root_path = (std::filesystem::path(output_path) / "assets").generic_string();
			asset_db_info.type = Vadon::Core::FileDatabaseType::FILESYSTEM;

			asset_db_handle = file_system.create_database(asset_db_info);
		}

		const std::vector<ResourceInfo> resource_list = m_database.get_resource_list(Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Scene::Resource>());

		Vadon::Scene::ResourceSystem& engine_resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		Vadon::Core::RawFileDataBuffer file_data_buffer;

		// TODO: implement some kind of dependency tracking to allow export to identify which resources will actually be needed!
		// - OR assume all are needed by default, and implement "include/exclude" metadata

		// TODO2: resources should be unloaded after this, except the ones currently in use while editing
		for (const ResourceInfo& current_editor_resource_info : resource_list)
		{
			file_data_buffer.clear();

			const Vadon::Scene::ResourceID& current_resource_id = current_editor_resource_info.info.id;
			Vadon::Scene::ResourceHandle resource_handle = engine_resource_system.load_resource_base(current_resource_id);
			VADON_ASSERT(resource_handle.is_valid() == true, "Failed to load resource!");

			Vadon::Utilities::Serializer::Instance binary_serializer = Vadon::Utilities::Serializer::create_serializer(file_data_buffer, Vadon::Utilities::Serializer::Type::BINARY, Vadon::Utilities::Serializer::Mode::WRITE);
			if (binary_serializer->initialize() == false)
			{
				Vadon::Core::Logger::log_error("Editor resource system: failed to initialize export serializer!\n");
				continue;
			}
			if (engine_resource_system.save_resource(*binary_serializer, resource_handle) == false)
			{
				Vadon::Core::Logger::log_error("Editor resource system: failed to serialize resource!\n");
				continue;
			}
			if (binary_serializer->finalize() == false)
			{
				Vadon::Core::Logger::log_error("Editor resource system: failed to finalize export serializer!\n");
				continue;
			}

			Vadon::Core::FileInfo file_info;
			file_info.offset = 0; // TODO: allow for files to be "packaged" into one file
			file_info.size = static_cast<int>(file_data_buffer.size());

			// Use hex representation of resource UUID as the file name
			for (unsigned char current_byte : current_resource_id.data)
			{
				file_info.path += std::format("{:02x}", current_byte);
			}
			file_info.path += ".vdbin";

			// Add to asset library via the file DB
			file_system.add_existing_file(resource_db_handle, current_resource_id, file_info);
			if (file_system.save_file(resource_db_handle, current_resource_id, file_data_buffer) == false)
			{
				Vadon::Core::Logger::log_error("Editor resource system: failed to save resource to file!\n");
				continue;
			}

			const Vadon::Scene::ResourceInfo current_resource_info = engine_resource_system.get_resource_info(resource_handle);
			if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Scene::FileResource>(), current_resource_info.type_id) == true)
			{
				// Resource points to a file, so we need to export that as well
				file_data_buffer.clear();

				// Add to the export database
				Vadon::Core::FileInfo asset_file_info;
				asset_file_info.offset = 0; // TODO: allow for files to be "packaged" into one file
				asset_file_info.size = 0; // FIXME: get the file size!

				// Same path as resource, but different DB root
				asset_file_info.path = file_info.path;

				file_system.add_existing_file(asset_db_handle, current_resource_id, file_info);

				// Copy from asset library to export destination
				const Vadon::Core::FileDatabaseHandle src_asset_db_handle = m_database.get_database(EditorResourceDatabaseImpl::FileDatabaseType::ASSET_FILE);
				if (file_system.copy_file(src_asset_db_handle, current_resource_id, asset_db_handle, current_resource_id) == false)
				{
					Vadon::Core::Logger::log_error("Editor resource system: failed to copy asset file!\n");
					continue;
				}
			}
		}

		// Remove the databases
		file_system.remove_database(resource_db_handle);
		file_system.remove_database(asset_db_handle);

		return true;
	}
}