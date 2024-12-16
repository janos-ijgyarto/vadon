#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/Resource/Resource.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/Scene/SceneSystem.hpp>
#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <format>

namespace VadonEditor::Model
{
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
		return internal_create_resource(resource_handle);
	}

	Resource* ResourceSystem::import_resource(const ResourcePath& path)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		Vadon::Scene::ResourceHandle imported_resource_handle = resource_system.import_resource(path);
		if (imported_resource_handle.is_valid() == false)
		{
			engine_core.log_error("Editor resource system: failed to import resource!\n");
			return nullptr;
		}

		return internal_create_resource(imported_resource_handle);
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
		std::string extensions_string = ".vdsc,.vdrc";

		const std::vector<ResourcePath> resource_files = file_system.get_files_of_type(ResourcePath{ .root_directory = project_manager.get_active_project().root_dir_handle}, extensions_string, true);
		for (const ResourcePath& current_file_path : resource_files)
		{
			all_valid &= resource_system.import_resource(current_file_path).is_valid();
		}

		return all_valid;
	}

	const Resource* ResourceSystem::find_resource(Vadon::Scene::ResourceHandle resource_handle) const
	{
		// TODO: should we make sure the handle is still valid?
		auto resource_it = m_resource_lookup.find(resource_handle.handle.to_uint());
		if (resource_it == m_resource_lookup.end())
		{
			return nullptr;
		}

		return &resource_it->second;
	}

	Resource* ResourceSystem::internal_create_resource(Vadon::Scene::ResourceHandle resource_handle)
	{
		Resource* resource = find_resource(resource_handle);
		if (resource != nullptr)
		{
			return resource;
		}

		// Resource not yet registered, so we create it
		const EditorResourceID new_resource_id = m_resource_id_counter++;
		auto resource_it = m_resource_lookup.emplace(resource_handle.handle.to_uint(), Resource(m_editor, resource_handle, new_resource_id)).first;
		return &resource_it->second;
	}
}