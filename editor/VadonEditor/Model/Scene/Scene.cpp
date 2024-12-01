#include <VadonEditor/Model/Scene/Scene.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>
#include <VadonEditor/Model/Scene/Entity.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Scene/SceneSystem.hpp>

namespace VadonEditor::Model
{
	void Scene::set_path(const Vadon::Scene::ResourcePath& path)
	{
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		resource_system.set_resource_path(m_handle, path);

		notify_modified();
		update_info();
	}

	bool Scene::save()
	{
		if(m_modified == false)
		{
			// Nothing to do
			return true;
		}

		// Must have a valid root entity to save
		if (is_open() == false)
		{
			m_editor.get_engine_core().log_error("Cannot save Scene that is not opened!\n");
			return false;
		}

		Vadon::Scene::SceneSystem& scene_system = m_editor.get_engine_core().get_system<Vadon::Scene::SceneSystem>();
		Vadon::ECS::World& ecs_world = m_editor.get_system<ModelSystem>().get_ecs_world();

		if (scene_system.package_scene_data(m_handle, ecs_world, m_root_entity->get_handle()) == false)
		{
			m_editor.get_engine_core().log_error("Failed to package Scene data!\n");
			return false;
		}

		if (scene_system.save_scene(m_handle) == false)
		{
			m_editor.get_engine_core().log_error("Failed to save Scene!\n");
			return false;
		}

		clear_modified();
		return true;
	}

	bool Scene::load()
	{
		Vadon::Scene::SceneSystem& scene_system = m_editor.get_engine_core().get_system<Vadon::Scene::SceneSystem>();
		return scene_system.load_scene(m_handle);
	}

	Entity* Scene::add_new_entity(Entity& parent)
	{
		// Scene can only modify its own contents
		if (is_owner_of_entity(parent) == false)
		{
			m_editor.get_engine_core().log_error("Scene cannot modify Entities of other scenes!\n");
			return nullptr;
		}

		return internal_add_new_entity(&parent);
	}

	Entity* Scene::instantiate_sub_scene(Vadon::Scene::SceneHandle scene_handle, Entity& parent)
	{
		// Should not try to instantiate itself within itself!
		if (scene_handle == m_handle)
		{
			m_editor.get_engine_core().log_error("Scene cannot add itself as sub-scene!\n");
			return nullptr;
		}

		Vadon::Scene::SceneSystem& scene_system = m_editor.get_engine_core().get_system<Vadon::Scene::SceneSystem>();
		if (scene_system.is_scene_dependent(m_handle, scene_handle) == true)
		{
			m_editor.get_engine_core().log_error("Cannot instantiate sub-scene that is dependent on this scene!\n");
			return nullptr;
		}

		// Scene can only modify its own contents
		if (is_owner_of_entity(parent) == false)
		{
			m_editor.get_engine_core().log_error("Cannot add sub-scene to Entity not owned by this scene!\n");
			return nullptr;
		}

		SceneSystem& model_scene_system = m_editor.get_system<Model::ModelSystem>().get_scene_system();
		Scene* sub_scene = model_scene_system.get_scene(scene_handle);

		if (sub_scene == nullptr)
		{
			m_editor.get_engine_core().log_error("Cannot find Scene object for sub-scene!\n");
			return nullptr;
		}

		Vadon::ECS::EntityHandle sub_scene_root_handle = sub_scene->instantiate(true);
		if (sub_scene_root_handle.is_valid() == false)
		{
			m_editor.get_engine_core().log_error("Failed to instantiate sub-scene!\n");
			return nullptr;
		}

		return instantiate_scene_recursive(sub_scene_root_handle, &parent);
	}

	bool Scene::remove_entity(Entity& entity)
	{
		if (is_owner_of_entity(entity) == false)
		{
			m_editor.get_engine_core().log_error("Scene cannot modify Entities of other scenes!\n");
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
		// Make sure scene is loaded
		if (load() == false)
		{
			return Vadon::ECS::EntityHandle();
		}

		Vadon::ECS::World& ecs_world = m_editor.get_system<ModelSystem>().get_ecs_world();
		Vadon::Scene::SceneSystem& scene_system = m_editor.get_engine_core().get_system<Vadon::Scene::SceneSystem>();
		return scene_system.instantiate_scene(m_handle, ecs_world, is_sub_scene);
	}

	Scene::Scene(Core::Editor& editor, Vadon::Scene::SceneHandle scene_handle)
		: m_editor(editor)
		, m_handle(scene_handle)
		, m_root_entity(nullptr)
		, m_modified(false)
	{

	}

	bool Scene::initialize()
	{
		update_info();

		if (m_info.path.is_valid() == false)
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

	void Scene::update_info()
	{
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		m_info = resource_system.get_resource_info(m_handle);
	}

	Entity* Scene::internal_add_new_entity(Entity* parent)
	{
		Vadon::ECS::World& ecs_world = m_editor.get_system<ModelSystem>().get_ecs_world();
		Vadon::ECS::EntityHandle new_entity_handle = ecs_world.get_entity_manager().create_entity();

		Entity* new_entity = internal_create_entity(new_entity_handle, parent);
		new_entity->set_name("Entity");

		return new_entity;
	}
	
	Entity* Scene::internal_create_entity(Vadon::ECS::EntityHandle entity_handle, Entity* parent)
	{
		SceneSystem& model_scene_system = m_editor.get_system<Model::ModelSystem>().get_scene_system();
		Entity* new_entity = new Entity(m_editor, entity_handle, model_scene_system.get_new_entity_id(), this);

		Vadon::ECS::World& ecs_world = m_editor.get_system<ModelSystem>().get_ecs_world();
		Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();

		new_entity->set_name(entity_manager.get_entity_name(entity_handle));

		if (parent != nullptr)
		{
			// Add to parent
			parent->add_child(new_entity);
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
		if (m_root_entity != nullptr)
		{
			m_editor.get_engine_core().log_error("Scene already has root Entity!\n");
			return;
		}
		Vadon::ECS::EntityHandle root_entity_handle = instantiate(false);
		m_root_entity = instantiate_scene_recursive(root_entity_handle);
	}

	void Scene::clear_contents()
	{
		// Remove the root
		internal_remove_entity(m_root_entity);
	}
}