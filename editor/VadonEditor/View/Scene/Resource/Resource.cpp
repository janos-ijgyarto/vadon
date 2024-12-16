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

	Vadon::Scene::ResourceHandle SelectResourceDialog::get_selected_resource() const
	{
		if (m_resource_list_box.has_valid_selection() == true)
		{
			return m_resource_list[m_resource_list_box.selected_item];
		}

		return Vadon::Scene::ResourceHandle();
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
		std::vector<Model::ResourceInfo> resource_info_list = resource_system.get_resource_list(m_resource_type);

		for (const Model::ResourceInfo& current_resource_info : resource_info_list)
		{
			if (current_resource_info.info.path.is_valid() == false)
			{
				continue;
			}

			m_resource_list_box.items.push_back(current_resource_info.info.path.path);
			m_resource_list.push_back(current_resource_info.handle);
		}
	}

	ResourceEditor::ResourceEditor(Core::Editor& editor)
		: m_editor(editor)
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
				dev_gui.add_text(m_path);
				for (auto& current_property : m_property_editors)
				{
					if (current_property->render(dev_gui) == true)
					{
						// Property edited, update the resource
						// FIXME: optimize this somehow?
						active_resource->edit_property(current_property->get_name(), current_property->get_value());
						current_property->set_value(active_resource->get_property(current_property->get_name()));
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
		m_path = resource->get_info().path.path;
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