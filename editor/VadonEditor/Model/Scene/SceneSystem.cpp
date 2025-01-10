#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/Scene/SceneSystem.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/Registry.hpp>

#include <format>

namespace
{
	Vadon::Utilities::TypeID get_scene_type_id()
	{
		return Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Scene::Scene>();
	}
}

namespace VadonEditor::Model
{
	Scene* SceneSystem::create_scene()
	{
		ResourceSystem& editor_resource_system = m_editor.get_system<ModelSystem>().get_resource_system();
		Resource* scene_resource = editor_resource_system.create_resource(get_scene_type_id());
		if (scene_resource == nullptr)
		{
			m_editor.get_engine_core().log_error("Editor scene system: failed to create scene resource!\n");
			return nullptr;
		}

		return internal_get_scene(scene_resource);
	}

	Scene* SceneSystem::get_scene(ResourceID scene_id)
	{
		ResourceSystem& editor_resource_system = m_editor.get_system<ModelSystem>().get_resource_system();

		Resource* scene_resource = editor_resource_system.get_resource(scene_id);
		if (scene_resource == nullptr)
		{
			// TODO: error?
			return nullptr;
		}

		return internal_get_scene(scene_resource);
	}

	Scene* SceneSystem::get_scene(Vadon::Scene::SceneHandle scene_handle)
	{
		ResourceSystem& editor_resource_system = m_editor.get_system<ModelSystem>().get_resource_system();
		Resource* scene_resource = editor_resource_system.get_resource(scene_handle);
		if (scene_resource == nullptr)
		{
			m_editor.get_engine_core().log_error("Editor scene system: failed to get scene resource!\n");
			return nullptr;
		}

		return internal_get_scene(scene_resource);
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
			m_editor.get_engine_core().log_error("Editor scene system: failed to load scene!\n");
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

		if (scene->get_path().is_valid() == false)
		{
			// New scene closed without saving, so we delete it
			internal_remove_scene(scene);
		}
	}

	void SceneSystem::remove_scene(Scene* scene)
	{
		scene->clear_contents();
		internal_remove_scene(scene);
	}

	std::vector<ResourceInfo> SceneSystem::get_scene_list() const
	{
		ResourceSystem& editor_resource_system = m_editor.get_system<ModelSystem>().get_resource_system();
		return editor_resource_system.get_database().get_resource_list(get_scene_type_id());
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
		// TODO: register scene file extension!
		return true;
	}

	Scene* SceneSystem::find_scene(Resource* resource)
	{
		// TODO: should we make sure the handle is still valid?
		auto scene_it = m_scene_lookup.find(resource);
		if (scene_it == m_scene_lookup.end())
		{
			return nullptr;
		}

		return &scene_it->second;
	}

	Scene* SceneSystem::internal_get_scene(Resource* resource)
	{
		Scene* scene = find_scene(resource);
		if (scene != nullptr)
		{
			return scene;
		}

		// Scene not yet registered, so we create it
		// Make sure the resource is actually a scene
		ResourceSystem& editor_resource_system = m_editor.get_system<ModelSystem>().get_resource_system();
		const Vadon::Scene::ResourceInfo scene_resource_info = (resource->is_loaded() == true) ? resource->get_info() : editor_resource_system.get_database().find_resource_info(resource->get_id()).info;
		if (scene_resource_info.type_id != get_scene_type_id())
		{
			m_editor.get_engine_core().log_error("Editor scene system: selected resource is not a scene!\n");
			return nullptr;
		}

		auto scene_it = m_scene_lookup.emplace(resource, Scene(m_editor, resource)).first;
		Scene& new_scene = scene_it->second;
		if (new_scene.initialize() == false)
		{
			m_editor.get_engine_core().log_error("Scene system: failed to initialize Scene object!\n");
			// TODO: remove from lookup!
			return nullptr;
		}

		return &new_scene;
	}

	void SceneSystem::internal_remove_scene(Scene* scene)
	{
		Resource* scene_resource = scene->m_resource;
		m_scene_lookup.erase(scene_resource);

		// TODO: use refcounting so we only remove the scene if nothing else is using it?
		ResourceSystem& editor_resource_system = m_editor.get_system<ModelSystem>().get_resource_system();
		editor_resource_system.remove_resource(scene_resource);
	}
}