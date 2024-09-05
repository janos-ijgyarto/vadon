#ifndef VADONEDITOR_VIEW_SCENE_ECS_ENTITY_HPP
#define VADONEDITOR_VIEW_SCENE_ECS_ENTITY_HPP
#include <VadonEditor/View/Scene/ECS/Component.hpp>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::View
{
	class EntityEditor
	{
	private:
		EntityEditor(Core::Editor& editor);

		void draw(UI::Developer::GUISystem& dev_gui);
		void set_selected_entity(Model::Entity* entity);
		void reset();

		void update_labels();
		void reset_components();

		Core::Editor& m_editor;

		Model::Entity* m_entity = nullptr;
		std::string m_sub_scene_label;
		std::string m_entity_path;

		UI::Developer::Window m_window;

		UI::Developer::InputText m_name_input;
		std::vector<ComponentEditor> m_component_editors;

		UI::Developer::ChildWindow m_component_tree; // Using tree to make components collapsable

		UI::Developer::Button m_add_component_button;
		AddComponentDialog m_add_component_dialog;

		friend class SceneTreeWindow;
	};
}
#endif