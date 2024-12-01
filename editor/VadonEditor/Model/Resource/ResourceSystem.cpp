#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

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
		return get_resource(new_resource_handle);
	}

	Resource* ResourceSystem::get_resource(Vadon::Scene::ResourceHandle resource_handle)
	{
		return internal_create_resource(resource_handle);
	}

	std::vector<ResourceInfo> ResourceSystem::get_resource_list(Vadon::Utilities::TypeID resource_type) const
	{
		std::vector<ResourceInfo> result;

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		std::vector<Vadon::Scene::ResourceHandle> resource_list = resource_system.find_resources_of_type(resource_type);

		for (Vadon::Scene::ResourceHandle current_resource_handle : resource_list)
		{
			const Vadon::Scene::ResourceInfo current_resource_info = resource_system.get_resource_info(current_resource_handle);
			result.emplace_back(current_resource_info, current_resource_handle);
		}

		return result;
	}

	ResourceSystem::ResourceSystem(Core::Editor& editor)
		: m_editor(editor)
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

		if (resource_system.import_resource_library(project_manager.get_active_project().root_dir_handle) == false)
		{
			// TODO: warning/error?
		}

		return true;
	}

	Resource* ResourceSystem::find_resource(Vadon::Scene::ResourceHandle resource_handle)
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
		auto resource_it = m_resource_lookup.emplace(resource_handle.handle.to_uint(), Resource(m_editor, resource_handle)).first;
		Resource& new_resource = resource_it->second;
		if (new_resource.initialize() == false)
		{
			m_editor.get_engine_core().log_error("Resource system: failed to initialize resource!\n");
			return nullptr;
		}

		return &new_resource;
	}
}