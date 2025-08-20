#include <VadonEditor/View/Scene/Resource/Resource.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>

#include <VadonEditor/View/ViewSystem.hpp>

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

	ResourceEditor::ResourceEditor(Core::Editor& editor)
		: m_editor(editor)
		, m_last_resource(nullptr)
	{
		m_window.title = "Resource Editor";
		m_window.open = false;

		m_save_button.label = "Save";
	}

	void ResourceEditor::draw(UI::Developer::GUISystem& dev_gui)
	{
		if (dev_gui.begin_window(m_window) == true)
		{
			ViewSystem& view_system = m_editor.get_system<ViewSystem>();
			ViewModel& view_model = view_system.get_view_model();

			Model::Resource* active_resource = view_model.get_active_resource();
			update_selected_resource(active_resource);
			if (active_resource != nullptr)
			{
				dev_gui.push_id(active_resource->get_editor_id());
				dev_gui.add_text_wrapped(m_path);
				for (auto& current_property : m_property_editors)
				{
					if (current_property->render(dev_gui) == true)
					{
						// Property edited, update the resource
						const Vadon::Utilities::Property& property_data = current_property->get_property();
						active_resource->edit_property(property_data.name, property_data.value);

						// Re-enter the value from the property into the editor (in case something changed it, e.g constraints)
						// FIXME: this will cause redundant refreshes of the UI!
						// TODO: make proper use of the "modified" flag and only update once explicitly requested (and only the modified elements)?
						current_property->set_value(active_resource->get_property(property_data.name));
					}
				}
				if (dev_gui.draw_button(m_save_button) == true)
				{
					active_resource->save();
				}
				dev_gui.pop_id();
			}
		}
		dev_gui.end_window();
	}

	void ResourceEditor::update_selected_resource(Model::Resource* resource)
	{
		if (resource == m_last_resource)
		{
			return;
		}

		m_last_resource = resource;
		if (resource == nullptr)
		{
			return;
		}

		update_labels(resource);
		reset_properties(resource);
	}

	void ResourceEditor::update_labels(Model::Resource* resource)
	{
		Model::ResourceSystem& resource_system = m_editor.get_system<Model::ModelSystem>().get_resource_system();
		const Model::ResourceInfo* resource_info = resource_system.get_database().find_resource_info(resource->get_id());
		VADON_ASSERT(resource_info != nullptr, "Resource not found!");
		if (resource_info->path.empty() == false)
		{
			m_path = resource_info->path;
		}
		else
		{
			m_path.clear();
		}
	}

	void ResourceEditor::reset_properties(Model::Resource* resource)
	{
		m_property_editors.clear();
		const Vadon::Utilities::PropertyList resource_properties = resource->get_properties();

		for (const Vadon::Utilities::Property& current_property : resource_properties)
		{
			m_property_editors.emplace_back(PropertyEditor::create_property_editor(m_editor, current_property));
		}
	}
}