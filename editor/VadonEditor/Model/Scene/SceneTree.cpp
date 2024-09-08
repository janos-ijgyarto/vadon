#include <VadonEditor/Model/Scene/SceneTree.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Scene/Entity.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/ECS/World/World.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Scene/SceneSystem.hpp>
#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <format>

namespace VadonEditor::Model
{
	void SceneTree::add_entity(Entity* parent)
	{
		Vadon::ECS::World& world = m_editor.get_system<ModelSystem>().get_ecs_world();
		Vadon::ECS::EntityHandle new_entity_handle = world.get_entity_manager().create_entity();

		Entity* new_entity = new Entity(m_editor, new_entity_handle, ++m_entity_id_counter);
		new_entity->set_name("Entity");

		if (parent != nullptr)
		{
			// Add to parent
			parent->add_child(new_entity);
		}
		else
		{
			// Add to scene root
			if (m_current_scene.root_entity != nullptr)
			{
				m_current_scene.root_entity->add_child(new_entity);
			}
			else
			{
				m_current_scene.root_entity = new_entity;
			}
		}

		notify_scene_modified();
	}

	void SceneTree::remove_entity(Entity* entity)
	{
		if (m_current_scene.root_entity == entity)
		{
			m_current_scene.root_entity = nullptr;
		}

		delete entity;
		notify_scene_modified();
	}

	bool SceneTree::instantiate_sub_scene(Entity* parent, Vadon::Scene::ResourceHandle scene_handle)
	{
		using Logger = Vadon::Core::Logger;
		
		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();

		if (engine_core.get_system<Vadon::Scene::ResourceSystem>().load_resource(scene_handle) == false)
		{
			Logger::log_error("Unable to load scene!\n");
			return false;
		}

		Vadon::Scene::SceneSystem& scene_system = engine_core.get_system<Vadon::Scene::SceneSystem>();
		if (scene_system.is_scene_dependent(m_current_scene.scene, scene_handle) == true)
		{
			Logger::log_error("Cannot instantiate sub-scene that has dependency toward current scene!\n");
			return false;
		}

		Vadon::ECS::EntityHandle root_entity_handle = scene_system.instantiate_scene(scene_handle, m_editor.get_system<ModelSystem>().get_ecs_world(), true);

		Entity* root_entity = instantiate_scene_recursive(root_entity_handle, nullptr);

		if (root_entity == nullptr)
		{
			// Something went wrong?
			return false;
		}

		parent->add_child(root_entity);

		notify_scene_modified();
		return true;
	}

	bool SceneTree::new_scene(std::string_view name)
	{
		// TODO: make getting a unique name a feature of the scene system?
		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		Vadon::Scene::ResourcePath scene_path;
		scene_path.path = std::string(name) + ".vdsc";
		scene_path.root_directory = m_editor.get_system<Core::ProjectManager>().get_active_project().root_dir_handle;
		if (resource_system.find_resource(scene_path).is_valid() == true)
		{
			Vadon::Core::Logger::log_error(std::format("Scene tree error: cannot create new scene at \"{}\" as it already exists!\n", scene_path.path));
			return false;
		}

		// Clean up previous scene
		// NOTE: it's the UI's job to prompt for unsaved scene. If we get this command, we discard any unsaved data
		clear_scene();

		m_current_scene.scene = engine_core.get_system<Vadon::Scene::SceneSystem>().create_scene();

		if (m_current_scene.scene.is_valid() == false)
		{
			// Something went wrong
			Vadon::Core::Logger::log_error("Scene tree error: failed to create new scene!\n");
			return false;
		}
		
		// FIXME: add helper function to scene system to get scene object?
		Vadon::Scene::Scene* scene = resource_system.get_resource<Vadon::Scene::Scene>(m_current_scene.scene);
		scene->name = name;

		resource_system.set_resource_path(m_current_scene.scene, scene_path);

		scene_changes_cleared();
		return true;
	}

	bool SceneTree::save_scene()
	{
		using Logger = Vadon::Core::Logger;
		if (m_current_scene.is_modified == false)
		{
			// Nothing to do
			// TODO: log?
			return true;
		}

		if (m_current_scene.root_entity == nullptr)
		{
			Logger::log_error("Scene tree error: unable to save scene with no root entity!\n");
			return false;
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Scene::SceneSystem& scene_system = engine_core.get_system<Vadon::Scene::SceneSystem>();

		// First write the entity data into the scene
		scene_system.set_scene_data(m_current_scene.scene, m_editor.get_system<ModelSystem>().get_ecs_world(), m_current_scene.root_entity->m_entity_handle);

		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
		if (resource_system.export_resource(m_current_scene.scene) == false)
		{
			return false;
		}

		scene_changes_cleared();
		return true;
	}

	bool SceneTree::load_scene(Vadon::Scene::ResourceHandle scene_handle)
	{
		using Logger = Vadon::Core::Logger;
		if (scene_handle.is_valid() == false)
		{
			Logger::log_error("Invalid scene handle!\n");
			return false;
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		// Prevent loading scene that is already loaded
		if (m_current_scene.scene == scene_handle)
		{
			Logger::log_error("Scene already loaded!\n");
			return false;
		}

		if (resource_system.load_resource(scene_handle) == false)
		{
			Logger::log_error("Unable to load scene!\n");
			return false;
		}

		// Clean up previous scene
		clear_scene();

		Vadon::Scene::SceneSystem& scene_system = engine_core.get_system<Vadon::Scene::SceneSystem>();
		Vadon::ECS::EntityHandle root_entity_handle = scene_system.instantiate_scene(scene_handle, m_editor.get_system<ModelSystem>().get_ecs_world(), false);
		m_current_scene.root_entity = instantiate_scene_recursive(root_entity_handle, nullptr);

		if (m_current_scene.root_entity == nullptr)
		{
			// Something went wrong?
			return false;
		}

		m_current_scene.scene = scene_handle;
		scene_changes_cleared();

		return true;
	}

	Scene SceneTree::get_scene_info(Vadon::Scene::ResourceHandle scene_handle) const
	{
		Scene scene_info;
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();

		scene_info.scene_handle = scene_handle;
		scene_info.resource_path = resource_system.get_resource_path(scene_handle);

		return scene_info;
	}

	SceneList SceneTree::get_scene_list() const
	{
		SceneList result;
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		for (Vadon::Scene::ResourceHandle scene_handle : resource_system.find_resources_of_type<Vadon::Scene::Scene>())
		{
			Scene& added_scene = result.emplace_back();
			added_scene.scene_handle = scene_handle;
			added_scene.resource_path = resource_system.get_resource_path(scene_handle);
		}

		return result;
	}

	SceneTree::SceneTree(Core::Editor& editor) :
		m_editor(editor)
	{

	}

	bool SceneTree::initialize()
	{
		// TODO: anything?
		return true;
	}

	Entity* SceneTree::instantiate_scene_recursive(Vadon::ECS::EntityHandle entity_handle, Entity* parent)
	{
		Vadon::ECS::World& ecs_world = m_editor.get_system<ModelSystem>().get_ecs_world();
		Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();

		Entity* new_entity = new Entity(m_editor, entity_handle, ++m_entity_id_counter, parent);
		new_entity->m_name = entity_manager.get_entity_name(entity_handle);

		const Vadon::ECS::EntityList child_entities = entity_manager.get_children(entity_handle);
		new_entity->m_children.reserve(child_entities.size());

		// FIXME: should we instead just avoid making these child nodes altogether?
		const Vadon::Scene::SceneComponent* scene_component = ecs_world.get_component_manager().get_component<Vadon::Scene::SceneComponent>(new_entity->get_handle());
		if (scene_component != nullptr)
		{
			if (scene_component->root_scene.is_valid())
			{
				new_entity->m_sub_scene = get_scene_info(scene_component->root_scene);
			}
			
			new_entity->m_sub_scene_child = scene_component->parent_scene.is_valid();
		}

		for (const Vadon::ECS::EntityHandle& current_child_entity : child_entities)
		{
			Entity* child_entity = instantiate_scene_recursive(current_child_entity, new_entity);
			new_entity->m_children.push_back(child_entity);
		}

		return new_entity;
	}

	void SceneTree::clear_scene()
	{
		if (m_current_scene.root_entity != nullptr)
		{
			delete m_current_scene.root_entity;
			m_current_scene.root_entity = nullptr;
		}
	}
}