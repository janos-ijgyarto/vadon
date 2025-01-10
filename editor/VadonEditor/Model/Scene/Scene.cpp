#include <VadonEditor/Model/Scene/Scene.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/Resource.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>
#include <VadonEditor/Model/Scene/Entity.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Scene/SceneSystem.hpp>

namespace VadonEditor::Model
{
	Vadon::Scene::SceneHandle Scene::get_handle() const
	{
		return Vadon::Scene::SceneHandle::from_resource_handle(m_resource->get_handle());
	}

	Vadon::Scene::SceneID Scene::get_id() const
	{
		return Vadon::Scene::SceneID::from_resource_id(m_resource->get_id());
	}

	Vadon::Scene::ResourceInfo Scene::get_info() const
	{
		return m_resource->get_info();
	}

	Vadon::Core::FileSystemPath Scene::get_path() const
	{
		return m_resource->get_path();
	}

	bool Scene::save()
	{
		if (is_modified() == false)
		{
			// Nothing to do
			return true;
		}

		if (is_open() == false)
		{
			Vadon::Core::Logger::log_error("Editor scene: cannot save scene that has not been opened!\n");
			return false;
		}

		// First package the scene data
		if (package_scene_data() == false)
		{
			return false;
		}

		// Attempt to save the resource
		if (m_resource->save() == false)
		{
			m_editor.get_engine_core().log_error("Editor scene: failed to save Scene!\n");
			return false;
		}

		return true;
	}

	bool Scene::save_as(const Vadon::Core::FileSystemPath& path)
	{
		if (is_open() == false)
		{
			Vadon::Core::Logger::log_error("Editor scene: cannot save scene that has not been opened!\n");
			return false;
		}

		// First package the scene data
		if (package_scene_data() == false)
		{
			return false;
		}

		// Attempt to save the resource
		return m_resource->save_as(path);
	}

	bool Scene::load()
	{
		if (is_loaded() == true)
		{
			// Nothing to do
			return true;
		}
		return m_resource->load();
	}

	bool Scene::is_loaded() const
	{
		return m_resource->is_loaded();
	}

	Entity* Scene::add_new_entity(Entity& parent)
	{
		// Scene can only modify its own contents
		if (is_owner_of_entity(parent) == false)
		{
			m_editor.get_engine_core().log_error("Editor scene: cannot modify Entities of other scenes!\n");
			return nullptr;
		}

		return internal_add_new_entity(&parent);
	}

	Entity* Scene::instantiate_sub_scene(Scene* scene, Entity& parent)
	{
		// Should not try to instantiate itself within itself!
		if (scene->get_id() == get_id())
		{
			m_editor.get_engine_core().log_error("Editor scene: cannot add itself as sub-scene!\n");
			return nullptr;
		}

		// Scene can only modify its own contents
		if (is_owner_of_entity(parent) == false)
		{
			m_editor.get_engine_core().log_error("Editor scene: cannot add sub-scene to Entity not owned by this scene!\n");
			return nullptr;
		}

		if (is_scene_dependent(scene->get_id()) == true)
		{
			m_editor.get_engine_core().log_error("Editor scene: cannot instantiate sub-scene that is dependent on this scene!\n");
			return nullptr;
		}

		Vadon::ECS::EntityHandle sub_scene_root_handle = scene->instantiate(true);
		if (sub_scene_root_handle.is_valid() == false)
		{
			m_editor.get_engine_core().log_error("Editor scene: failed to instantiate sub-scene!\n");
			return nullptr;
		}

		// Instantiate without parent, then add to parent (to get unique name, etc.)
		Entity* sub_scene_entity = instantiate_scene_recursive(sub_scene_root_handle, nullptr);
		parent.add_child(sub_scene_entity);

		return sub_scene_entity;
	}

	bool Scene::remove_entity(Entity& entity)
	{
		if (is_owner_of_entity(entity) == false)
		{
			m_editor.get_engine_core().log_error("Editor scene: cannot modify Entities of other scenes!\n");
			return false;
		}

		if (m_root_entity == &entity)
		{
			init_empty_scene();
		}

		internal_remove_entity(&entity);
		notify_modified();

		return true;
	}

	Vadon::ECS::EntityHandle Scene::instantiate(bool is_sub_scene)
	{
		if (load() == false)
		{
			m_editor.get_engine_core().log_error("Editor scene: failed to load scene for instantiation!\n");
			return Vadon::ECS::EntityHandle();
		}

		Vadon::ECS::World& ecs_world = m_editor.get_system<ModelSystem>().get_ecs_world();
		Vadon::Scene::SceneSystem& scene_system = m_editor.get_engine_core().get_system<Vadon::Scene::SceneSystem>();

		Vadon::ECS::EntityHandle instantiated_entity_handle = scene_system.instantiate_scene(get_handle(), ecs_world, is_sub_scene);
		if (instantiated_entity_handle.is_valid() == false)
		{
			m_editor.get_engine_core().log_error("Editor scene: failed to instantiate scene!\n");
		}

		return instantiated_entity_handle;
	}

	bool Scene::is_modified() const
	{
		return m_resource->is_modified();
	}

	void Scene::notify_modified()
	{
		m_resource->notify_modified();
	}

	void Scene::clear_modified()
	{
		m_resource->clear_modified();
	}

	Scene::Scene(Core::Editor& editor, Resource* scene_resource)
		: m_editor(editor)
		, m_resource(scene_resource)
		, m_root_entity(nullptr)
	{

	}

	bool Scene::initialize()
	{
		if (m_resource->get_path().is_valid() == false)
		{
			init_empty_scene();
		}

		return true;
	}

	void Scene::init_empty_scene()
	{
		// Replace root with a new empty entity
		m_root_entity = internal_add_new_entity(nullptr);
	}

	Entity* Scene::internal_add_new_entity(Entity* parent)
	{
		Vadon::ECS::World& ecs_world = m_editor.get_system<ModelSystem>().get_ecs_world();
		Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();

		Vadon::ECS::EntityHandle new_entity_handle = entity_manager.create_entity();
		entity_manager.set_entity_name(new_entity_handle, "Entity");

		// Create with no parent to get default initialization of editor Entity
		Entity* new_entity = internal_create_entity(new_entity_handle, nullptr);
		if (parent != nullptr)
		{
			// Add to parent here to both update ECS state and ensure unique name
			parent->add_child(new_entity);
		}

		return new_entity;
	}
	
	Entity* Scene::internal_create_entity(Vadon::ECS::EntityHandle entity_handle, Entity* parent)
	{
		// NOTE: at this point, all the state in the editor Entity is initialized from and validated against the ECS backend
		// Subsequent operations are all handled as edits
		SceneSystem& model_scene_system = m_editor.get_system<Model::ModelSystem>().get_scene_system();
		Entity* new_entity = new Entity(m_editor, entity_handle, model_scene_system.get_new_entity_id(), this);

		Vadon::ECS::World& ecs_world = m_editor.get_system<ModelSystem>().get_ecs_world();
		Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();

		new_entity->m_name = entity_manager.get_entity_name(entity_handle);

		if (parent != nullptr)
		{
			const Vadon::ECS::EntityHandle parent_handle = entity_manager.get_entity_parent(entity_handle);
			assert(parent_handle == parent->m_entity_handle);
		
			// Add to parent
			parent->m_children.push_back(new_entity);
			new_entity->m_parent = parent;
		}

		notify_modified();

		return new_entity;
	}

	void Scene::internal_remove_entity(Entity* entity)
	{
		m_editor.get_system<ModelSystem>().get_ecs_world().remove_entity(entity->get_handle());
		delete entity;
	}

	bool Scene::is_owner_of_entity(Entity& entity) const
	{
		return entity.get_owning_scene() == this;
	}

	bool Scene::is_scene_dependent(Vadon::Scene::SceneID scene_id) const
	{
		// TODO: this will load scenes to do the check, need to make sure we unload scenes that aren't referenced afterward!
		Vadon::Scene::SceneSystem& scene_system = m_editor.get_engine_core().get_system<Vadon::Scene::SceneSystem>();
		return scene_system.is_scene_dependent(get_id(), scene_id);
	}

	Entity* Scene::instantiate_scene_recursive(Vadon::ECS::EntityHandle entity_handle, Entity* parent)
	{
		Vadon::ECS::World& ecs_world = m_editor.get_system<ModelSystem>().get_ecs_world();
		Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();

		Entity* instantiated_scene_entity = internal_create_entity(entity_handle, parent);

		const Vadon::ECS::EntityList child_entities = entity_manager.get_children(entity_handle);
		instantiated_scene_entity->m_children.reserve(child_entities.size());

		SceneSystem& model_scene_system = m_editor.get_system<Model::ModelSystem>().get_scene_system();

		// FIXME: should we instead just avoid making these child nodes altogether?
		const Vadon::Scene::SceneComponent* scene_component = ecs_world.get_component_manager().get_component<Vadon::Scene::SceneComponent>(instantiated_scene_entity->get_handle());
		if (scene_component != nullptr)
		{
			if (scene_component->root_scene.is_valid())
			{
				Scene* sub_scene = model_scene_system.get_scene(scene_component->root_scene);
				instantiated_scene_entity->m_sub_scene = sub_scene;
			}
		}

		for (const Vadon::ECS::EntityHandle& current_child_entity : child_entities)
		{
			instantiate_scene_recursive(current_child_entity, instantiated_scene_entity);
		}

		return instantiated_scene_entity;
	}

	void Scene::instantiate_from_root()
	{
		Vadon::ECS::EntityHandle root_entity_handle = instantiate(false);
		m_root_entity = instantiate_scene_recursive(root_entity_handle);
	}

	void Scene::clear_contents()
	{
		// Remove the root
		internal_remove_entity(m_root_entity);
	}

	bool Scene::package_scene_data()
	{
		Vadon::Scene::SceneSystem& scene_system = m_editor.get_engine_core().get_system<Vadon::Scene::SceneSystem>();
		Vadon::ECS::World& ecs_world = m_editor.get_system<ModelSystem>().get_ecs_world();

		if (scene_system.package_scene_data(get_handle(), ecs_world, m_root_entity->get_handle()) == false)
		{
			m_editor.get_engine_core().log_error("Editor scene: failed to package Scene data!\n");
			return false;
		}

		return true;
	}
}