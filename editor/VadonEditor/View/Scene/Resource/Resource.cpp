#include <VadonEditor/View/Scene/Resource/Resource.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>

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
}