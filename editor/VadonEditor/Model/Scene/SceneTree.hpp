#ifndef VADONEDITOR_MODEL_SCENE_SCENETREE_HPP
#define VADONEDITOR_MODEL_SCENE_SCENETREE_HPP
#include <Vadon/Core/Object/Property.hpp>
#include <Vadon/Scene/Scene.hpp>
namespace Vadon::Scene
{
	class Node;
}
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	class SceneTree
	{
	public:
		Vadon::Scene::SceneHandle get_current_scene() const { return m_current_scene; }
		Vadon::Scene::Node* get_root() const { return m_current_scene_root; }

		// FIXME: Should we use pointers, or provide some indirect access?
		void add_node(std::string_view class_id, Vadon::Scene::Node* parent);
		Vadon::Core::ObjectPropertyList get_node_properties(Vadon::Scene::Node& node) const;
		Vadon::Core::Variant get_node_property_value(Vadon::Scene::Node& node, std::string_view property_name) const;
		void edit_node_property(Vadon::Scene::Node& node, std::string_view property_name, const Vadon::Core::Variant& value);
		void delete_node(Vadon::Scene::Node* node);

		bool save_scene();
		bool load_scene();
	private:
		SceneTree(Core::Editor& editor);

		bool initialize();

		Core::Editor& m_editor;
		Vadon::Scene::Node* m_current_scene_root = nullptr;

		Vadon::Scene::SceneHandle m_current_scene;

		// TODO: mapping between scenes and the instanced root nodes?

		friend class ModelSystem;
	};
}
#endif