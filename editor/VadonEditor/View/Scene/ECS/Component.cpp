#include <VadonEditor/View/Scene/ECS/Component.hpp>

#include <VadonEditor/Model/Scene/Entity.hpp>

namespace VadonEditor::View
{
	UI::Developer::Dialog::Result AddComponentDialog::internal_draw(UI::Developer::GUISystem& dev_gui)
	{
		Result result = Result::NONE;
		dev_gui.draw_list_box(m_component_type_list);
		if (has_valid_option() == false)
		{
			dev_gui.begin_disabled();
		}
		if (dev_gui.draw_button(m_add_button) == true)
		{
			result = Result::ACCEPTED;
			close();
		}
		if (has_valid_option() == false)
		{
			dev_gui.end_disabled();
		}
		dev_gui.same_line();
		if (dev_gui.draw_button(m_cancel_button) == true)
		{
			result = Result::CANCELLED;
			close();
		}

		return result;
	}

	bool AddComponentDialog::initialize(Model::Entity& entity)
	{
		// Rebuild component type list
		m_component_type_list.items.clear();
		m_component_type_ids.clear();

		const Vadon::Utilities::TypeInfoList component_info_list = entity.get_available_component_list();
		for (const Vadon::Utilities::TypeInfo& current_component_type_info : component_info_list)
		{
			m_component_type_list.items.push_back(current_component_type_info.name);
			m_component_type_ids.push_back(current_component_type_info.id);
		}

		return false;
	}

	AddComponentDialog::AddComponentDialog()
		: Dialog("Add Component")
	{
		m_add_button.label = "Add";
		m_cancel_button.label = "Cancel";

		m_component_type_list.label = "Component types";
	}

	ComponentEditor::ComponentEditor()
		: m_type_id(Vadon::Utilities::TypeID::INVALID)
	{
		m_remove_button.label = "Remove component";
	}

	void ComponentEditor::initialize(Core::Editor& editor, Model::Entity& entity)
	{
		const VadonEditor::Model::Component component_data = entity.get_component_data(m_type_id);
		m_name = component_data.name;

		for (const Vadon::Utilities::Property& current_property : component_data.properties)
		{
			m_property_editors.emplace_back(PropertyEditor::create_property_editor(editor, current_property, false));
		}
	}

	bool ComponentEditor::draw(VadonApp::UI::Developer::GUISystem& dev_gui, Model::Entity& entity)
	{
		// TODO: context menus and/or buttons for adding/removing components
		bool removed = false;
		if (dev_gui.push_tree_node(m_name) == true)
		{
			for (auto& current_property : m_property_editors)
			{
				if (current_property->render(dev_gui) == true)
				{
					// Property edited, send update to scene tree
					const Vadon::Utilities::Property& property_data = current_property->get_property();
					entity.edit_component_property(m_type_id, property_data.name, property_data.value);

					// Re-enter the value from the property into the editor (in case something changed it, e.g constraints)
					// FIXME: this will cause redundant refreshes of the UI!
					// TODO: make proper use of the "modified" flag and only update once explicitly requested (and only the modified elements)?
					current_property->set_value(entity.get_component_property(m_type_id, property_data.name));
				}
			}

			// Can only add or remove components on entities that are not sub-scenes
			if (entity.is_sub_scene() == false)
			{
				// FIXME: use a closeable child window instead?
				if (dev_gui.draw_button(m_remove_button) == true)
				{
					entity.remove_component(m_type_id);
					removed = true;
				}
			}
			dev_gui.pop_tree_node();
		}
		return removed;
	}
}