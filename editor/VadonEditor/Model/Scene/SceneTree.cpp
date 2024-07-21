#include <VadonEditor/Model/Scene/SceneTree.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Scene/Entity.hpp>

#include <Vadon/ECS/World/World.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Scene/SceneSystem.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <filesystem>
#include <fstream>

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
			if (m_current_scene_root != nullptr)
			{
				m_current_scene_root->add_child(new_entity);
			}
			else
			{
				m_current_scene_root = new_entity;
			}
		}
	}

	void SceneTree::remove_entity(Entity* entity)
	{
		if (m_current_scene_root == entity)
		{
			m_current_scene_root = nullptr;
		}

		delete entity;
	}

	bool SceneTree::save_scene()
	{
		using Logger = Vadon::Core::Logger;
		Vadon::Scene::SceneSystem& scene_system = m_editor.get_engine_core().get_system<Vadon::Scene::SceneSystem>();

		// First write the entity data into the scene
		scene_system.set_scene_data(m_current_scene, m_editor.get_system<ModelSystem>().get_ecs_world(), m_current_scene_root->m_entity_handle);

		// Save the scene to JSON
		std::vector<std::byte> scene_data_buffer;
		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(scene_data_buffer, Vadon::Utilities::Serializer::Type::JSON, false);
		if (serializer->initialize() == false)
		{
			return false;
		}

		if (scene_system.serialize_scene(m_current_scene, *serializer) == false)
		{
			Logger::log_error("Error saving scene!\n");
			return false;
		}

		if (serializer->finalize() == false)
		{
			Logger::log_error(serializer->get_last_error() + '\n');
			return false;
		}

		const Vadon::Scene::SceneInfo scene_info = scene_system.get_scene_info(m_current_scene);

		// FIXME: implement file system to encapsulate this!
		std::ofstream scene_file(scene_info.name + ".vdsc");
		if (scene_file.is_open() == false)
		{
			Logger::log_error("Unable to open scene file!\n");
			return false;
		}

		scene_file.write(reinterpret_cast<char*>(scene_data_buffer.data()), scene_data_buffer.size());
		scene_file.close();
		return true;
	}

	bool VadonEditor::Model::SceneTree::load_scene()
	{
		using Logger = Vadon::Core::Logger;

		// FIXME: implement proper scene management, project file, etc.
		Vadon::Scene::SceneInfo scene_info;
		scene_info.name = "TestScene";

		Vadon::Scene::SceneSystem& scene_system = m_editor.get_engine_core().get_system<Vadon::Scene::SceneSystem>();

		if (m_current_scene.is_valid() == false)
		{
			m_current_scene = scene_system.create_scene(scene_info);
		}

		if (m_current_scene.is_valid() == false)
		{
			Logger::log_error("Error while creating scene!\n");
			return false;
		}

		const std::string scene_file_path = scene_info.name + ".vdsc";
		if (std::filesystem::exists(scene_file_path) == true)
		{
			std::ifstream scene_file(scene_file_path, std::ios::binary);
			if (scene_file.good() == false)
			{
				Logger::log_error("Unable to open scene file!\n");
				return false;
			}

			std::vector<std::byte> scene_data_buffer;
			{
				scene_file.seekg(0, std::ios::end);
				std::streampos file_size = scene_file.tellg();
				scene_file.seekg(0, std::ios::beg);

				scene_data_buffer.resize(file_size);
				
				scene_file.read(reinterpret_cast<char*>(scene_data_buffer.data()), file_size);
			}

			scene_file.close();

			Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(scene_data_buffer, Vadon::Utilities::Serializer::Type::JSON, true);
			if (serializer->initialize() == false)
			{
				Logger::log_error(serializer->get_last_error() + '\n');
				return false;
			}

			if (scene_system.serialize_scene(m_current_scene, *serializer) == false)
			{
				return false;
			}

			if (serializer->finalize() == false)
			{
				Logger::log_error(serializer->get_last_error() + '\n');
				return false;
			}

			Vadon::ECS::EntityHandle root_entity_handle = scene_system.instantiate_scene(m_current_scene, m_editor.get_system<ModelSystem>().get_ecs_world());
			m_current_scene_root = instantiate_scene_recursive(root_entity_handle, nullptr);

			if (m_current_scene_root == nullptr)
			{
				return false;
			}
		}
		else
		{
			Logger::log_message("No scene file found, starting with empty scene.\n");
		}

		return true;
	}

	Vadon::Utilities::TypeInfoList SceneTree::get_component_type_list() const
	{
		Vadon::Utilities::TypeInfoList component_type_list;

		const std::vector<uint32_t> component_type_ids = Vadon::ECS::ComponentRegistry::get_component_types();
		for(uint32_t current_type_id : component_type_ids)
		{
			component_type_list.push_back(Vadon::Utilities::TypeRegistry::get_type_info(current_type_id));
		}

		return component_type_list;
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
		Vadon::ECS::EntityManager& entity_manager = m_editor.get_system<ModelSystem>().get_ecs_world().get_entity_manager();

		Entity* new_entity = new Entity(m_editor, entity_handle, ++m_entity_id_counter, parent);
		new_entity->m_name = entity_manager.get_entity_name(entity_handle);

		const Vadon::ECS::EntityList child_entities = entity_manager.get_children(entity_handle);
		new_entity->m_children.reserve(child_entities.size());

		for (const Vadon::ECS::EntityHandle& current_child_entity : child_entities)
		{
			Entity* child_entity = instantiate_scene_recursive(current_child_entity, new_entity);
			new_entity->m_children.push_back(child_entity);
		}

		return new_entity;
	}
}