#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/View/ViewSystem.hpp>

#include <Vadon/Core/File/FileSystem.hpp>
#include <Vadon/ECS/World/World.hpp>
#include <Vadon/Scene/SceneSystem.hpp>
#include <Vadon/Utilities/TypeInfo/Registry.hpp>

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
			Vadon::Core::Logger::log_error("Editor scene system: failed to create scene resource!\n");
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
			Vadon::Core::Logger::log_error("Editor scene system: failed to get scene resource!\n");
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
			Vadon::Core::Logger::log_error("Editor scene system: failed to load scene!\n");
			return;
		}

		scene->instantiate_from_root();

		// FIXME: use events instead of directly notifying view!
		m_editor.get_system<View::ViewSystem>().get_view_model().scene_opened(scene);
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

		if (scene->get_path().empty() == true)
		{
			// New scene closed without saving, so we delete it
			internal_remove_scene(scene);
		}

		// FIXME: use events instead of directly notifying view!
		m_editor.get_system<View::ViewSystem>().get_view_model().scene_closed(scene);
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

	void SceneSystem::add_entity_event_callback(EntityEventCallback callback)
	{
		m_entity_callbacks.push_back(callback);
	}

	void SceneSystem::add_component_event_callback(ComponentEventCallback callback)
	{
		m_component_callbacks.push_back(callback);
	}

	void SceneSystem::entity_added(const Entity& entity)
	{
		dispatch_entity_event(EntityEvent{ .type = EntityEventType::ADDED, .entity = entity.get_handle() });
	}

	void SceneSystem::entity_removed(const Entity& entity)
	{
		dispatch_entity_event(EntityEvent{ .type = EntityEventType::REMOVED, .entity = entity.get_handle() });
	}

	void SceneSystem::component_added(const Entity& owner, Vadon::Utilities::TypeID component_id)
	{
		dispatch_component_event(ComponentEvent{ .type = ComponentEventType::ADDED, .owner = owner.get_handle(), .component_type = component_id });
	}

	void SceneSystem::component_edited(const Entity& owner, Vadon::Utilities::TypeID component_id)
	{
		dispatch_component_event(ComponentEvent{ .type = ComponentEventType::EDITED, .owner = owner.get_handle(), .component_type = component_id });
	}

	void SceneSystem::component_removed(const Entity& owner, Vadon::Utilities::TypeID component_id)
	{
		dispatch_component_event(ComponentEvent{ .type = ComponentEventType::REMOVED, .owner = owner.get_handle(), .component_type = component_id });
	}

	const Scene* SceneSystem::find_entity_scene(Vadon::ECS::EntityHandle entity) const
	{
		Vadon::ECS::EntityHandle root_entity = m_editor.get_system<ModelSystem>().get_ecs_world().get_entity_manager().get_entity_root(entity);
		for (auto& scene_pair : m_scene_lookup)
		{
			Entity* root_editor_entity = scene_pair.second.get_root();
			if((root_editor_entity != nullptr) && (root_editor_entity->get_handle() == root_entity))
			{
				return &scene_pair.second;
			}
		}

		return nullptr;
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
		const Vadon::Scene::ResourceInfo scene_resource_info = (resource->is_loaded() == true) ? resource->get_info() : editor_resource_system.get_database().find_resource_info(resource->get_id())->info;
		if (scene_resource_info.type_id != get_scene_type_id())
		{
			Vadon::Core::Logger::log_error("Editor scene system: selected resource is not a scene!\n");
			return nullptr;
		}

		auto scene_it = m_scene_lookup.emplace(resource, Scene(m_editor, resource)).first;
		Scene& new_scene = scene_it->second;
		if (new_scene.initialize() == false)
		{
			Vadon::Core::Logger::log_error("Scene system: failed to initialize Scene object!\n");
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

	void SceneSystem::dispatch_entity_event(const EntityEvent& event)
	{
		for (const EntityEventCallback& current_callback : m_entity_callbacks)
		{
			current_callback(event);
		}
	}

	void SceneSystem::dispatch_component_event(const ComponentEvent& event)
	{
		for (const ComponentEventCallback& current_callback : m_component_callbacks)
		{
			current_callback(event);
		}
	}
}