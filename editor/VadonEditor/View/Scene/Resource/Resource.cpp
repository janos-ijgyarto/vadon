#include <VadonEditor/View/Scene/Resource/Resource.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>

#include <VadonEditor/View/ViewSystem.hpp>

#include <Vadon/Utilities/TypeInfo/Registry.hpp>

namespace VadonEditor::View
{
	SelectResourceDialog::SelectResourceDialog(Core::Editor& editor, std::string_view title)
		: Dialog(title)
		, m_editor(editor)
	{
		m_select_button.label = "Select";
		m_cancel_button.label = "Cancel";

		m_resource_list_box.label = "Resources";
	}

	Model::ResourceID SelectResourceDialog::get_selected_resource() const
	{
		if (m_resource_list_box.has_valid_selection() == true)
		{
			return m_resource_list[m_resource_list_box.selected_item];
		}

		return Model::ResourceID();
	}

	UI::Developer::Dialog::Result SelectResourceDialog::internal_draw(UI::Developer::GUISystem& dev_gui)
	{
		Result result = Result::NONE;

		dev_gui.draw_list_box(m_resource_list_box);

		const bool valid_selection = m_resource_list_box.has_valid_selection();
		if (valid_selection == false)
		{
			dev_gui.begin_disabled();
		}
		if (dev_gui.draw_button(m_select_button) == true)
		{
			result = Result::ACCEPTED;
			close();
		}
		if (valid_selection == false)
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

	void SelectResourceDialog::on_open()
	{
		m_resource_list.clear();

		m_resource_list_box.items.clear();
		m_resource_list_box.deselect();

		Model::ResourceSystem& resource_system = m_editor.get_system<Model::ModelSystem>().get_resource_system();
		std::vector<Model::ResourceInfo> resource_info_list = resource_system.get_database().get_resource_list(m_resource_type);

		for (const Model::ResourceInfo& current_resource_info : resource_info_list)
		{
			if (current_resource_info.path.empty() == true)
			{
				continue;
			}

			m_resource_list_box.items.push_back(current_resource_info.path);
			m_resource_list.push_back(current_resource_info.info.id);
		}
	}

	ResourceEditorWidget::ResourceEditorWidget(Core::Editor& editor, bool read_only)
		: m_editor(editor)
		, m_read_only(read_only)
		, m_resource(nullptr)
	{
		m_child_window.int_id = 1;
		m_child_window.border = true;
	}

	void ResourceEditorWidget::set_resource(Model::Resource* resource)
	{
		if (m_resource == resource)
		{
			return;
		}

		m_child_window.int_id = resource != nullptr ? (resource->get_editor_id() + 1) : 1; // Offset by 1 to ensure it's not 0
		m_resource = resource;

		reset_properties(resource);
	}

	bool ResourceEditorWidget::draw(UI::Developer::GUISystem& dev_gui, const Vadon::Math::Vector2& size)
	{
		bool edited = false;
		if (m_resource != nullptr)
		{
			dev_gui.push_style_var(UI::Developer::GUIStyleVar::CHILD_BORDER_SIZE, 0.5f);
			m_child_window.size = size;

			if (dev_gui.begin_child_window(m_child_window) == true)
			{
				for (auto& current_property : m_property_editors)
				{
					if (current_property->render(dev_gui) == true)
					{
						// Property edited, update the resource
						const Vadon::Utilities::Property& property_data = current_property->get_property();
						m_resource->edit_property(property_data.name, property_data.value);

						// Re-enter the value from the property into the editor (in case something changed it, e.g constraints)
						// FIXME: this will cause redundant refreshes of the UI!
						// TODO: make proper use of the "modified" flag and only update once explicitly requested (and only the modified elements)?
						current_property->set_value(m_resource->get_property(property_data.name));

						edited = true;
					}
				}
			}
			dev_gui.end_child_window();
			dev_gui.pop_style_var();
		}

		return edited;
	}

	void ResourceEditorWidget::reset_properties(Model::Resource* resource)
	{
		m_property_editors.clear();
		if (resource == nullptr)
		{
			return;
		}

		const Vadon::Utilities::PropertyList resource_properties = resource->get_properties();
		for (const Vadon::Utilities::Property& current_property : resource_properties)
		{
			m_property_editors.emplace_back(PropertyEditor::create_property_editor(m_editor, current_property, m_read_only));
		}
	}

	ResourceEditorWindow::ResourceEditorWindow(Core::Editor& editor)
		: m_editor(editor)
		, m_editor_widget(editor, false)
	{
		m_window.title = "Resource Editor";
		m_window.open = false;

		m_save_button.label = "Save";

		update_label(nullptr);
	}

	void ResourceEditorWindow::draw(UI::Developer::GUISystem& dev_gui)
	{
		if (dev_gui.begin_window(m_window) == true)
		{
			ViewSystem& view_system = m_editor.get_system<ViewSystem>();
			ViewModel& view_model = view_system.get_view_model();

			Model::Resource* active_resource = view_model.get_active_resource();

			if (active_resource != m_editor_widget.get_resource())
			{
				m_editor_widget.set_resource(active_resource);
				update_label(active_resource);
			}

			dev_gui.add_text_wrapped(m_label);

			Vadon::Math::Vector2 editor_widget_size = dev_gui.get_available_content_region();

			// TODO: use separator, ensure enough space is left for it!
			const VadonApp::UI::Developer::GUIStyle gui_style = dev_gui.get_style();
			editor_widget_size.y -= dev_gui.calculate_text_size(m_save_button.label).y + gui_style.frame_padding.y * 2 + 5.0f;

			if (m_editor_widget.draw(dev_gui, editor_widget_size) == true)
			{
				// TODO: set a "modified" flag to notify user if they have unsaved changes?
			}

			if (active_resource == nullptr)
			{
				dev_gui.begin_disabled(true);
			}
			if (dev_gui.draw_button(m_save_button) == true)
			{
				active_resource->save();
			}
			if (active_resource == nullptr)
			{
				dev_gui.end_disabled();
			}
		}
		dev_gui.end_window();
	}

	void ResourceEditorWindow::update_label(Model::Resource* resource)
	{
		if (resource != nullptr)
		{
			std::string resource_path = resource->get_path();
			if (resource_path.empty() == true)
			{
				resource_path = "UNSAVED";
			}

			const Vadon::Utilities::TypeInfo resource_type_info = Vadon::Utilities::TypeRegistry::get_type_info(resource->get_info().type_id);
			m_label = std::format("{} ({})", resource_path, resource_type_info.name);
		}
		else
		{
			m_label = "<NONE>";
		}
	}
}