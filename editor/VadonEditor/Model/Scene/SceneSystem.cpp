#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/Scene/SceneSystem.hpp>

#include <format>

namespace VadonEditor::Model
{
	Scene* SceneSystem::create_scene()
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Scene::SceneSystem& engine_scene_system = engine_core.get_system<Vadon::Scene::SceneSystem>();

		Vadon::Scene::SceneHandle new_scene_handle = engine_scene_system.create_scene();
		return get_scene(new_scene_handle);
	}

	Scene* SceneSystem::get_scene(Vadon::Scene::SceneHandle scene_handle)
	{
		return internal_create_scene(scene_handle);
	}

	void SceneSystem::remove_scene(Scene* scene)
	{
		scene->clear_contents();
		internal_remove_scene(scene);
	}

	void SceneSystem::open_scene(Scene* scene)
	{
		if (scene->is_open() == true)
		{
			// Nothing to do
			return;
		}

		if (scene->load() == false)
		{
			m_editor.get_engine_core().log_error("Scene system: failed to load scene!\n");
			return;
		}

		scene->instantiate_from_root();
	}

	void SceneSystem::close_scene(Scene* scene)
	{
		if (scene->is_open() == false)
		{
			// Nothing to do
			return;
		}

		scene->clear_contents();
		scene->clear_modified();
				
		if (scene->get_info().path.is_valid() == false)
		{
			// New scene closed without saving, so we delete it
			internal_remove_scene(scene);
		}
	}

	std::vector<SceneInfo> SceneSystem::get_scene_list() const
	{
		std::vector<SceneInfo> scene_info_list;

		ResourceSystem& editor_resource_system = m_editor.get_system<ModelSystem>().get_resource_system();
		const std::vector<ResourceInfo> scene_resource_list = editor_resource_system.get_resource_list(Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Scene::Scene>());

		scene_info_list.reserve(scene_resource_list.size());
		for (const ResourceInfo& current_scene_resource : scene_resource_list)
		{
			scene_info_list.push_back(SceneInfo{ .info = current_scene_resource.info, .handle = current_scene_resource.handle });
		}

		return scene_info_list;
	}

	EntityID SceneSystem::get_new_entity_id()
	{
		return m_entity_id_counter++;
	}

	void SceneSystem::register_edit_callback(EditCallback callback)
	{
		m_edit_callbacks.push_back(callback);
	}

	void SceneSystem::entity_edited(const Entity& entity, Vadon::Utilities::TypeID component_id)
	{
		for (const EditCallback& current_callback : m_edit_callbacks)
		{
			current_callback(entity.get_handle(), component_id);
		}
	}

	SceneSystem::SceneSystem(Core::Editor& editor)
		: m_editor(editor)
		, m_entity_id_counter(0)
	{

	}

	bool SceneSystem::initialize()
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		resource_system.register_resource_file_extension(".vdsc");

		return true;
	}

	Scene* SceneSystem::find_scene(Vadon::Scene::SceneHandle scene_handle)
	{
		// TODO: should we make sure the handle is still valid?
		auto scene_it = m_scene_lookup.find(scene_handle.handle.to_uint());
		if (scene_it == m_scene_lookup.end())
		{
			return nullptr;
		}

		return &scene_it->second;
	}

	Scene* SceneSystem::internal_create_scene(Vadon::Scene::SceneHandle scene_handle)
	{
		Scene* scene = find_scene(scene_handle);
		if (scene != nullptr)
		{
			return scene;
		}

		// Scene not yet registered, so we create it
		auto scene_it = m_scene_lookup.emplace(scene_handle.handle.to_uint(), Scene(m_editor, scene_handle)).first;
		Scene& new_scene = scene_it->second;
		if (new_scene.initialize() == false)
		{
			m_editor.get_engine_core().log_error("Scene system: failed to initialize Scene object!\n");
			return nullptr;
		}

		return &new_scene;
	}

	void SceneSystem::internal_remove_scene(Scene* scene)
	{
		// TODO: also remove the scene from engine scene system?
		m_scene_lookup.erase(scene->get_handle().to_uint());
	}
}