#include <VadonEditor/View/Scene/ECS/Entity.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>

#include <VadonEditor/Model/Scene/Entity.hpp>
#include <VadonEditor/Model/Scene/Scene.hpp>

#include <VadonEditor/View/ViewSystem.hpp>

#include <format>

namespace VadonEditor::View
{
	EntityEditor::EntityEditor(Core::Editor& editor)
		: m_editor(editor)
	{
		m_window.title = "Entity Editor";
		m_window.open = false;

		m_name_input.label = "Entity name";

		m_component_tree.string_id = "ComponentTree";
		m_component_tree.border = true;

		m_add_component_button.label = "Add Component";
	}

	void EntityEditor::draw(UI::Developer::GUISystem& dev_gui)
	{
		if (dev_gui.begin_window(m_window) == true)
		{
			ViewSystem& view_system = m_editor.get_system<ViewSystem>();
			ViewModel& view_model = view_system.get_view_model();

			Model::Entity* active_entity = view_model.get_active_entity();
			update_selected_entity(active_entity);

			if (active_entity != nullptr)
			{
				dev_gui.push_id(active_entity->get_id());
				if (dev_gui.draw_input_text(m_name_input) == true)
				{
					active_entity->set_name(m_name_input.input);
					update_labels(active_entity);
				}
				dev_gui.add_text(m_entity_path);
				if (active_entity->is_sub_scene() == true)
				{
					dev_gui.add_text(m_sub_scene_label);
				}

				// Can only add or remove components on entities that are not sub-scenes
				if (active_entity->is_sub_scene() == false)
				{
					if (dev_gui.draw_button(m_add_component_button) == true)
					{
						m_add_component_dialog.initialize(*active_entity);
						m_add_component_dialog.open();
					}
				}

				if (m_add_component_dialog.draw(dev_gui) == VadonApp::UI::Developer::Dialog::Result::ACCEPTED)
				{
					active_entity->add_component(m_add_component_dialog.get_selected_component_type());
					reset_components(active_entity);
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
							component_removed |= current_component.draw(dev_gui, *active_entity);
						}
					}
					dev_gui.end_child_window();
					if (component_removed == true)
					{
						reset_components(active_entity);
					}
				}
				dev_gui.pop_id();
			}
		}
		dev_gui.end_window();
	}

	void EntityEditor::update_selected_entity(Model::Entity* entity)
	{
		if (entity == m_last_entity)
		{
			return;
		}

		m_last_entity = entity;
		if (entity == nullptr)
		{
			return;
		}

		if (entity->is_sub_scene() == true)
		{
			m_sub_scene_label.clear();
			std::format_to(std::back_inserter(m_sub_scene_label), "Instantiated scene: {}", entity->get_sub_scene()->get_path());
		}

		update_labels(entity);
		reset_components(entity);
	}

	void EntityEditor::update_labels(Model::Entity* entity)
	{
		m_name_input.input = entity->get_name();
		m_entity_path = "Path: " + entity->get_path();
	}

	void EntityEditor::reset_components(Model::Entity* entity)
	{
		m_component_editors.clear();

		const Vadon::ECS::ComponentIDList component_id_list = entity->get_component_types();
		for (Vadon::ECS::ComponentID current_component_id : component_id_list)
		{
			ComponentEditor& current_component_editor = m_component_editors.emplace_back();
			current_component_editor.m_type_id = current_component_id;

			current_component_editor.initialize(m_editor, *entity);
		}
	}
}