#ifndef VADONEDITOR_VIEW_SCENE_ECS_COMPONENT_HPP
#define VADONEDITOR_VIEW_SCENE_ECS_COMPONENT_HPP
#include <VadonEditor/View/Widgets/Property.hpp>
#include <VadonEditor/UI/Developer/Widgets.hpp>
#include <Vadon/ECS/Component/Component.hpp>
namespace VadonEditor::Model
{
	class Entity;
}
namespace VadonEditor::View
{
	class AddComponentDialog : public UI::Developer::Dialog
	{
	protected:
		Result internal_draw(UI::Developer::GUISystem& dev_gui) override;

		Vadon::ECS::ComponentID get_selected_component_type() const
		{
			return m_component_type_ids[m_component_type_list.selected_item];
		}

		bool has_valid_option() const
		{
			return (m_component_type_ids.empty() == false) && (m_component_type_list.has_valid_selection() == true);
		}
	private:
		AddComponentDialog();

		bool initialize(Model::Entity& entity);

		UI::Developer::ListBox m_component_type_list;
		Vadon::ECS::ComponentIDList m_component_type_ids;

		UI::Developer::Button m_add_button;
		UI::Developer::Button m_cancel_button;

		friend class EntityEditor;
	};

	class ComponentEditor
	{
	public:
		ComponentEditor();
	private:
		void initialize(Model::Entity& entity);
		bool draw(VadonApp::UI::Developer::GUISystem& dev_gui, Model::Entity& entity);

		Vadon::ECS::ComponentID m_type_id;
		std::string m_name;

		std::vector<PropertyEditor::Instance> m_property_editors;
		UI::Developer::GUISystem::ID m_property_editors_id = 0;

		UI::Developer::Button m_remove_button;

		friend class EntityEditor;
	};

}
#endif