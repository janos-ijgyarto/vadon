#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/Resource/Resource.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>

namespace VadonEditor::Model
{
	bool EditorResourceDatabase::save_resource(Vadon::Scene::ResourceSystem& resource_system, Vadon::Scene::ResourceHandle resource_handle)
	{
		const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);
		const ResourcePath resource_path = find_resource_path(resource_info.id);

		if (resource_path.is_valid() == false)
		{
			return false;
		}

		return internal_save_resource(resource_system, resource_handle, resource_path);
	}

	Vadon::Scene::ResourceHandle EditorResourceDatabase::load_resource(Vadon::Scene::ResourceSystem& resource_system, ResourceID resource_id)
	{
		const ResourcePath resource_path = find_resource_path(resource_id);
		if (resource_path.is_valid() == false)
		{
			return Vadon::Scene::ResourceHandle();
		}

		return internal_load_resource(resource_system, resource_path);
	}

	bool EditorResourceDatabase::save_resource(Vadon::Scene::ResourceHandle resource_handle, const ResourcePath& path)
	{
		return internal_save_resource(m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>(), resource_handle, path);
	}

	Vadon::Scene::ResourceHandle EditorResourceDatabase::load_resource(const ResourcePath& path)
	{
		return internal_load_resource(m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>(), path);
	}

	ResourceID EditorResourceDatabase::find_resource_id(const ResourcePath& path)
	{
		Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
		
		auto resource_id_it = m_resource_id_lookup.find(file_system.get_absolute_path(path));
		if (resource_id_it != m_resource_id_lookup.end())
		{
			return resource_id_it->second;
		}

		return ResourceID();
	}

	ResourcePath EditorResourceDatabase::find_resource_path(ResourceID resource_id)
	{
		auto resource_file_it = m_resource_file_lookup.find(resource_id);
		if (resource_file_it != m_resource_file_lookup.end())
		{
			return resource_file_it->second;
		}

		return ResourcePath();
	}
	
	ResourceID EditorResourceDatabase::import_resource(const ResourcePath& path)
	{
		// TODO: first check path
		// If not already present, load resource info from resource system
		// Cache ID and path
		return ResourceID();
	}

	bool EditorResourceDatabase::internal_save_resource(Vadon::Scene::ResourceSystem& resource_system, Vadon::Scene::ResourceHandle resource_handle, const ResourcePath& path)
	{
		// FIXME: support binary file serialization!
		// Solution: have file system create the appropriate serializer!
		Vadon::Core::FileSystem::RawFileDataBuffer resource_file_buffer;
		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::WRITE);

		if (serializer->initialize() == false)
		{
			resource_system.log_error("Editor resource serializer: failed to initialize serializer while saving resource!\n");
			return false;
		}

		if (resource_system.save_resource(*serializer, resource_handle) == false)
		{
			resource_system.log_error("Editor resource serializer: failed to serialize resource data!\n");
			return false;
		}

		if (serializer->finalize() == false)
		{
			resource_system.log_error("Editor resource serializer: failed to finalize serializer after saving resource!\n");
			return false;
		}

		Vadon::Core::FileSystem& file_system = resource_system.get_engine_core().get_system<Vadon::Core::FileSystem>();
		if (file_system.save_file(path, resource_file_buffer) == false)
		{
			resource_system.log_error("Editor resource serializer: failed to save resource data to file!\n");
			return false;
		}

		// TODO: update file lookups!

		return true;
	}

	Vadon::Scene::ResourceHandle EditorResourceDatabase::internal_load_resource(Vadon::Scene::ResourceSystem& resource_system, const ResourcePath& path)
	{
		Vadon::Core::FileSystem& file_system = resource_system.get_engine_core().get_system<Vadon::Core::FileSystem>();
		Vadon::Core::FileSystem::RawFileDataBuffer resource_file_buffer;
		if (file_system.load_file(path, resource_file_buffer) == false)
		{
			resource_system.log_error("Editor resource serializer: failed to load resource file!\n");
			return Vadon::Scene::ResourceHandle();
		}

		// FIXME: support binary file serialization!
		// Solution: have file system create the appropriate serializer!
		Vadon::Utilities::Serializer::Instance serializer_instance = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);

		if (serializer_instance->initialize() == false)
		{
			resource_system.log_error("Editor resource serializer: failed to initialize serializer while loading resource!\n");
			return Vadon::Scene::ResourceHandle();
		}

		Vadon::Scene::ResourceHandle loaded_resource_handle = resource_system.load_resource(*serializer_instance);
		if (loaded_resource_handle.is_valid() == false)
		{
			resource_system.log_error("Editor resource serializer: failed to load resource data!\n");
			return loaded_resource_handle;
		}

		if (serializer_instance->finalize() == false)
		{
			resource_system.log_error("Editor resource serializer: failed to finalize serializer after loading resource!\n");
		}

		// TODO: update file lookups!

		return loaded_resource_handle;
	}

	ResourcePath EditorResourceDatabase::find_resource_path(Vadon::Scene::ResourceID resource_id) const
	{
		auto resource_file_it = m_resource_file_lookup.find(resource_id);
		if (resource_file_it == m_resource_file_lookup.end())
		{
			return ResourcePath();
		}
		return resource_file_it->second;
	}

	Resource* ResourceSystem::create_resource(Vadon::Utilities::TypeID resource_type)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		Vadon::Scene::ResourceHandle new_resource_handle = resource_system.create_resource(resource_type);
		if (new_resource_handle.is_valid() == false)
		{
			engine_core.log_error("Editor resource system: failed to create resource!\n");
			return nullptr;
		}

		return get_resource(new_resource_handle);
	}

	Resource* ResourceSystem::get_resource(Vadon::Scene::ResourceHandle resource_handle)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		return internal_get_resource(resource_system.get_resource_info(resource_handle).id);
	}

	std::vector<ResourceInfo> ResourceSystem::get_resource_list(Vadon::Utilities::TypeID resource_type) const
	{
		std::vector<ResourceInfo> result;

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		const std::vector<Vadon::Scene::ResourceHandle> resource_list = resource_system.find_resources_of_type(resource_type);
		result.reserve(resource_list.size());

		for (Vadon::Scene::ResourceHandle current_resource_handle : resource_list)
		{
			const Vadon::Scene::ResourceInfo current_resource_info = resource_system.get_resource_info(current_resource_handle);
			result.emplace_back(current_resource_info, current_resource_handle);
		}

		return result;
	}

	void ResourceSystem::remove_resource(Resource* /*resource*/)
	{
		// TODO: implement refcounting or some other system to track when a resource should be unloaded/removed!
		// TODO2: remove from engine resource system!
	}

	ResourceSystem::ResourceSystem(Core::Editor& editor)
		: m_editor(editor)
		, m_resource_id_counter(0)
		, m_database(editor)
	{

	}

	bool ResourceSystem::initialize()
	{
		// TODO: anything?
		return true;
	}

	bool ResourceSystem::load_project_resources()
	{
		// Import all resources in the project
		Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		bool all_valid = true;
		Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();

		// FIXME: make this modular!
		// Scene system should load scene files!
		std::string extensions_string = ".vdsc,.vdrc";

		const std::vector<ResourcePath> resource_files = file_system.get_files_of_type(ResourcePath{ .root_directory = project_manager.get_active_project().root_dir_handle}, extensions_string, true);
		for (const ResourcePath& current_file_path : resource_files)
		{
			all_valid &= m_database.import_resource(current_file_path).is_valid();
		}

		return all_valid;
	}

	Resource* ResourceSystem::internal_get_resource(ResourceID resource_id)
	{
		auto resource_it = m_resource_lookup.find(resource_id);
		if (resource_it == m_resource_lookup.end())
		{
			// Resource not yet registered, so we create it
			const EditorResourceID new_resource_id = m_resource_id_counter++;
			auto resource_it = m_resource_lookup.emplace(resource_id, Resource(m_editor, resource_id, new_resource_id)).first;
		}

		return &resource_it->second;
	}
}