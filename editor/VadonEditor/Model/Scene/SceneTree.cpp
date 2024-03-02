#include <VadonEditor/Model/Scene/SceneTree.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <Vadon/Core/Object/ObjectSystem.hpp>

#include <Vadon/Scene/SceneSystem.hpp>
#include <Vadon/Scene/Node/Node.hpp>

#include <Vadon/Utilities/Serialization/JSON/JSONImpl.hpp>

#include <filesystem>
#include <fstream>

namespace VadonEditor::Model
{
	void SceneTree::add_node(std::string_view class_id, Vadon::Scene::Node* parent)
	{
		Vadon::Core::ObjectSystem& object_system = m_editor.get_engine_core().get_system<Vadon::Core::ObjectSystem>();
		Vadon::Core::Object* new_object = object_system.create_object(class_id);

		Vadon::Scene::Node* new_node = object_system.get_object_as<Vadon::Scene::Node>(*new_object);
		
		// Initially set name to class name
		new_node->set_name(object_system.get_class_info(class_id).name);
		
		if (parent != nullptr)
		{
			// Add to parent
			parent->add_child(new_node);
		}
		else
		{
			// Add to scene root
			if (m_current_scene_root != nullptr)
			{
				m_current_scene_root->add_child(new_node);
			}
			else
			{
				m_current_scene_root = new_node;
			}
		}
	}

	Vadon::Core::ObjectPropertyList SceneTree::get_node_properties(Vadon::Scene::Node& node) const
	{
		// TODO: filter to those that are meant to be editable?
		return m_editor.get_engine_core().get_system<Vadon::Core::ObjectSystem>().get_object_properties(node);
	}

	Vadon::Core::Variant SceneTree::get_node_property_value(Vadon::Scene::Node& node, std::string_view property_name) const
	{
		return m_editor.get_engine_core().get_system<Vadon::Core::ObjectSystem>().get_property(node, property_name);
	}

	void SceneTree::edit_node_property(Vadon::Scene::Node& node, std::string_view property_name, const Vadon::Core::Variant& value)
	{
		m_editor.get_engine_core().get_system<Vadon::Core::ObjectSystem>().set_property(node, property_name, value);
	}

	void SceneTree::delete_node(Vadon::Scene::Node* node)
	{
		if (node == m_current_scene_root)
		{
			m_current_scene_root = nullptr;
		}

		Vadon::Scene::Node* parent = node->get_parent();
		if (parent != nullptr)
		{
			parent->remove_child(node);
		}

		delete node;
	}

	bool SceneTree::save_scene()
	{
		Vadon::Scene::SceneSystem& scene_system = m_editor.get_engine_core().get_system<Vadon::Scene::SceneSystem>();
		Vadon::Core::Logger& logger = m_editor.get_engine_core().get_logger();

		// First write the node data into the scene
		scene_system.set_scene_data(m_current_scene, *m_current_scene_root);

		// Save the scene to JSON
		Vadon::Utilities::JSON scene_json;
		if (scene_system.save_scene(m_current_scene, scene_json) == false)
		{
			logger.error("Error saving scene!\n");
			return false;
		}

		const Vadon::Scene::SceneInfo scene_info = scene_system.get_scene_info(m_current_scene);

		// FIXME: implement file system to encapsulate this!
		std::ofstream scene_file(scene_info.name + ".vdsc");
		if (scene_file.is_open() == false)
		{
			logger.error("Unable to open scene file!\n");
			return false;
		}

		scene_file << scene_json.dump(4);
		scene_file.close();
		return true;
	}

	bool VadonEditor::Model::SceneTree::load_scene()
	{
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
			m_editor.get_engine_core().get_logger().error("Error while creating scene!\n");
			return false;
		}

		Vadon::Core::Logger& logger = m_editor.get_engine_core().get_logger();
		const std::string scene_file_path = scene_info.name + ".vdsc";
		if (std::filesystem::exists(scene_file_path) == true)
		{
			std::ifstream scene_file(scene_file_path);
			if (scene_file.is_open() == false)
			{
				logger.error("Unable to open scene file!\n");
				return false;
			}

			Vadon::Utilities::JSON scene_data_json;
			try
			{
				scene_data_json = Vadon::Utilities::JSON::parse(scene_file);
			}
			catch (const Vadon::Utilities::JSON::exception& exception)
			{
				logger.error(std::string(exception.what()) + '\n');
				return false;
			}

			scene_file.close();

			Vadon::Utilities::JSONReader scene_data_reader(scene_data_json);
			if (scene_system.load_scene(m_current_scene, scene_data_reader) == false)
			{
				return false;
			}

			m_current_scene_root = scene_system.instantiate_scene(m_current_scene);
			if (m_current_scene_root == nullptr)
			{
				return false;
			}
		}
		else
		{
			logger.log("No scene file found, starting with empty scene.\n");
		}

		return true;
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
}