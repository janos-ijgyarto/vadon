#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <Vadon/Core/File/FileSystem.hpp>
#include <Vadon/Scene/Resource/ResourceSystem.hpp>

namespace VadonEditor::Model
{
	Resource* ResourceSystem::get_resource(Vadon::Scene::ResourceHandle resource_handle)
	{
		// TODO: validate resource?
		auto resource_it = m_resource_lookup.find(resource_handle.handle.to_uint());
		if(resource_it == m_resource_lookup.end())
		{
			resource_it = m_resource_lookup.emplace(resource_handle.handle.to_uint(), Resource(m_editor, resource_handle)).first;
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		resource_system.load_resource(resource_handle);

		resource_it->second.update_name();

		return &resource_it->second;
	}

	std::vector<ResourceInfo> ResourceSystem::get_resource_list(Vadon::Utilities::TypeID resource_type) const
	{
		std::vector<ResourceInfo> result;

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();

		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		const std::vector<Vadon::Scene::ResourceHandle> resources = resource_system.find_resources_of_type(resource_type);

		for (Vadon::Scene::ResourceHandle current_resource : resources)
		{
			ResourceInfo& current_resource_info = result.emplace_back();
			current_resource_info.handle = current_resource;

			const Vadon::Scene::ResourcePath resource_path = resource_system.get_resource_path(current_resource);
			current_resource_info.path = file_system.get_absolute_path(Vadon::Core::FileSystem::Path{ .path = resource_path.path, .root = resource_path.root_directory });
		}

		return result;
	}

	ResourceSystem::ResourceSystem(Core::Editor& editor)
		: m_editor(editor)
	{

	}

	bool ResourceSystem::initialize()
	{
		// TODO: look up resources in project?
		return true;
	}
}