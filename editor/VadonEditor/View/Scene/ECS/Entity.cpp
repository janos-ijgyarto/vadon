#include <VadonEditor/View/Scene/ECS/Entity.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>

#include <VadonEditor/Model/Scene/Entity.hpp>
#include <VadonEditor/Model/Scene/SceneTree.hpp>

#include <format>

namespace VadonEditor::View
{
	EntityEditor::EntityEditor(Core::Editor& editor)
		: m_editor(editor)
	{
		m_window.title = "Entity Editor";
		m_window.open = false;

		m_name_input.label = "Entity name";

		m_component_tree.id = "ComponentTree";
		m_component_tree.border = true;

		m_add_component_button.label = "Add Component";
	}

	void EntityEditor::draw(UI::Developer::GUISystem& dev_gui)
	{
		if (dev_gui.begin_window(m_window) == true)
		{
			Model::ModelSystem& model = m_editor.get_system<Model::ModelSystem>();
			Model::SceneTree& scene_tree = model.get_scene_tree();

			if ((scene_tree.get_current_scene().is_valid() == true) && (m_entity != nullptr))
			{
				dev_gui.push_id(m_entity->get_id());
				if (dev_gui.draw_input_text(m_name_input) == true)
				{
					m_entity->set_name(m_name_input.input);
					update_labels();
				}
				dev_gui.add_text(m_entity_path);
				if (m_entity->is_sub_scene() == true)
				{
					dev_gui.add_text(m_sub_scene_label);
				}

				// Can only add or remove components on entities that are not sub-scenes
				if (m_entity->is_sub_scene() == false)
				{
					if (dev_gui.draw_button(m_add_component_button) == true)
					{
						m_add_component_dialog.initialize(*m_entity);
						m_add_component_dialog.open();
					}
				}

				if (m_add_component_dialog.draw(dev_gui) == VadonApp::UI::Developer::Dialog::Result::ACCEPTED)
				{
					m_entity->add_component(m_add_component_dialog.get_selected_component_type());
					reset_components();
				}

				if (m_component_editors.empty() == false)
				{
					// TODO: revise this so we show the property editors for only one component at a time
					// and reset the view whenever a component is added/removed
					bool component_removed = false;
					if (dev_gui.begin_child_window(m_component_tree) == true)
					{
						for (ComponentEditor& current_component : m_component_editors)
						{
							component_removed |= current_component.draw(dev_gui, *m_entity);
						}
					}
					dev_gui.end_child_window();
					if (component_removed == true)
					{
						reset_components();
					}
				}
				dev_gui.pop_id();
			}
		}
		dev_gui.end_window();
	}

	void EntityEditor::set_selected_entity(Model::Entity* entity)
	{
		if (m_entity == entity)
		{
			return;
		}

		m_entity = entity;
		if (m_entity == nullptr)
		{
			return;
		}

		if (m_entity->is_sub_scene() == true)
		{
			m_sub_scene_label.clear();
			std::format_to(std::back_inserter(m_sub_scene_label), "Instantiated scene: {}", m_entity->get_sub_scene_info().resource_path.path);
		}

		update_labels();
		reset_components();
	}

	void EntityEditor::reset()
	{
		set_selected_entity(nullptr);
	}

	void EntityEditor::update_labels()
	{
		m_name_input.input = m_entity->get_name();
		m_entity_path = "Path: " + m_entity->get_path();
	}

	void EntityEditor::reset_components()
	{
		m_component_editors.clear();

		const Vadon::ECS::ComponentIDList component_id_list = m_entity->get_component_types();
		for (Vadon::ECS::ComponentID current_component_id : component_id_list)
		{
			ComponentEditor& current_component_editor = m_component_editors.emplace_back();
			current_component_editor.m_type_id = current_component_id;

			current_component_editor.initialize(*m_entity);
		}
	}
}