#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/Resource/Resource.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>

namespace VadonEditor::Model
{
	ResourceSystem::EditorResourceDatabaseImpl::EditorResourceDatabaseImpl(Core::Editor& editor)
		: m_editor(editor)
	{
	}

	bool ResourceSystem::EditorResourceDatabaseImpl::save_resource(Vadon::Scene::ResourceSystem& resource_system, Vadon::Scene::ResourceHandle resource_handle)
	{
		const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);
		const ResourceInfo* editor_resource_info = internal_find_resource_info(resource_info.id);

		if (editor_resource_info == nullptr)
		{
			return false;
		}

		return internal_save_resource(resource_system, resource_handle, editor_resource_info->path);
	}

	Vadon::Scene::ResourceHandle ResourceSystem::EditorResourceDatabaseImpl::load_resource(Vadon::Scene::ResourceSystem& resource_system, ResourceID resource_id)
	{
		const ResourceInfo* resource_info = internal_find_resource_info(resource_id);
		if (resource_info == nullptr)
		{
			return Vadon::Scene::ResourceHandle();
		}

		Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
		Vadon::Core::FileSystem::RawFileDataBuffer resource_file_buffer;
		if (file_system.load_file(resource_info->path, resource_file_buffer) == false)
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

	ResourceID ResourceSystem::EditorResourceDatabaseImpl::find_resource_id(const ResourcePath& path) const
	{
		Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
		
		auto resource_id_it = m_resource_id_lookup.find(file_system.get_absolute_path(path));
		if (resource_id_it != m_resource_id_lookup.end())
		{
			return resource_id_it->second;
		}

		return ResourceID();
	}

	ResourceInfo ResourceSystem::EditorResourceDatabaseImpl::find_resource_info(ResourceID resource_id) const
	{
		const ResourceInfo* info = internal_find_resource_info(resource_id);
		if (info != nullptr)
		{
			return *info;
		}

		return ResourceInfo();
	}

	bool ResourceSystem::EditorResourceDatabaseImpl::save_resource_as(ResourceID resource_id, const ResourcePath& path)
	{
		if (internal_find_resource_info(resource_id) != nullptr)
		{
			Vadon::Core::Logger::log_error("Editor resource database: cannot save-as a resource that is already imported!\n");
			return false;
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		const Vadon::Scene::ResourceHandle resource_handle = resource_system.find_resource(resource_id);
		if (resource_handle.is_valid() == false)
		{
			Vadon::Core::Logger::log_error("Editor resource database: cannot save resource that isn't loaded!\n");
			return false;
		}

		if (internal_save_resource(resource_system, resource_handle, path) == false)
		{
			Vadon::Core::Logger::log_error("Editor resource database: failed to save resource!\n");
			return false;
		}

		// Success, add resource to DB
		internal_import_resource(resource_system.get_resource_info(resource_handle), path);
		return true;
	}
	
	ResourceID ResourceSystem::EditorResourceDatabaseImpl::import_resource(const ResourcePath& path)
	{
		const ResourceID resource_id = find_resource_id(path);
		if (resource_id.is_valid() == true)
		{
			// Resource already imported
			// TODO: notification?
			return resource_id;
		}

		// TODO: deduplicate parts shared with loading a resource!
		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();
		Vadon::Core::FileSystem::RawFileDataBuffer resource_file_buffer;
		if (file_system.load_file(path, resource_file_buffer) == false)
		{
			Vadon::Core::Logger::log_error("Editor resource database: failed to load resource file!\n");
			return ResourceID();
		}

		// FIXME: support binary file serialization!
		// Solution: have file system create the appropriate serializer!
		Vadon::Utilities::Serializer::Instance serializer_instance = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);

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
		
		const ResourceInfo* existing_resource_info = internal_find_resource_info(imported_resource_info.id);
		if (existing_resource_info != nullptr)
		{
			// TODO: show resource ID!
			Vadon::Core::Logger::log_error(std::format("Editor resource database: resource already imported with path \"{}\"!\n", existing_resource_info->path.path));
			return ResourceID();
		}

		internal_import_resource(imported_resource_info, path);

		return imported_resource_info.id;
	}

	std::vector<ResourceInfo> ResourceSystem::EditorResourceDatabaseImpl::get_resource_list(Vadon::Utilities::TypeID resource_type) const
	{
		std::vector<ResourceInfo> result;

		for (const auto& resource_info_pair : m_resource_file_lookup)
		{
			const Vadon::Scene::ResourceInfo& current_resource_info = resource_info_pair.second.info;
			if(Vadon::Utilities::TypeRegistry::is_base_of(resource_type, current_resource_info.type_id))
			{
				result.push_back(resource_info_pair.second);
			}
		}

		return result;
	}

	const ResourceInfo* ResourceSystem::EditorResourceDatabaseImpl::internal_find_resource_info(ResourceID resource_id) const
	{
		auto resource_file_it = m_resource_file_lookup.find(resource_id);
		if (resource_file_it != m_resource_file_lookup.end())
		{
			return &resource_file_it->second;
		}

		return nullptr;
	}

	void ResourceSystem::EditorResourceDatabaseImpl::internal_import_resource(const Vadon::Scene::ResourceInfo& resource_info, const ResourcePath& path)
	{
		m_resource_file_lookup[resource_info.id] = ResourceInfo{ .info = resource_info, .path = path };

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();
		m_resource_id_lookup[file_system.get_absolute_path(path)] = resource_info.id;
	}

	void ResourceSystem::EditorResourceDatabaseImpl::internal_remove_resource(ResourceID resource_id)
	{
		// Remove any previous path
		auto resource_file_it = m_resource_file_lookup.find(resource_id);
		if (resource_file_it != m_resource_file_lookup.end())
		{
			Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
			Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();
			
			m_resource_id_lookup.erase(file_system.get_absolute_path(resource_file_it->second.path));

			m_resource_file_lookup.erase(resource_file_it);
		}
	}

	bool ResourceSystem::EditorResourceDatabaseImpl::internal_save_resource(Vadon::Scene::ResourceSystem& resource_system, Vadon::Scene::ResourceHandle resource_handle, const ResourcePath& path)
	{

		// FIXME: support binary file serialization!
		// Solution: have file system create the appropriate serializer!
		Vadon::Core::FileSystem::RawFileDataBuffer resource_file_buffer;
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

		Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
		if (file_system.save_file(path, resource_file_buffer) == false)
		{
			resource_system.log_error("Editor resource database: failed to save resource data to file!\n");
			return false;
		}

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

	void ResourceSystem::remove_resource(Resource* /*resource*/)
	{
		// TODO: implement refcounting or some other system to track when a resource should be unloaded/removed!
		// TODO2: remove from engine resource system!
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
		// Import all resources in the project
		Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();

		bool all_valid = true;

		// FIXME: make this modular!
		// Scene system should load scene files!
		std::string extensions_string = ".vdsc,.vdrc";

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

		const std::vector<ResourcePath> resource_files = file_system.get_files_of_type(ResourcePath{ .root_directory = project_manager.get_active_project().root_dir_handle}, extensions_string, true);
		for (const ResourcePath& current_file_path : resource_files)
		{
			all_valid &= m_database.import_resource(current_file_path).is_valid();
		}

		return all_valid;
	}
}