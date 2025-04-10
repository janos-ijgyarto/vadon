#include <VadonEditor/View/Project/Project.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <format>

namespace VadonEditor::View
{
	UI::Developer::Dialog::Result NewProjectDialog::internal_draw(UI::Developer::GUISystem& dev_gui)
	{
		Result result = Result::NONE;

		if (m_file_browser.draw(dev_gui) == UI::Developer::Dialog::Result::ACCEPTED)
		{
			m_root_path = m_file_browser.get_selected_file().path;
			if (m_root_path.empty() == true)
			{
				m_root_path = m_file_browser.get_current_path();
			}
		}

		dev_gui.draw_input_text(m_input_name);
		dev_gui.add_text("Root path: ");

		dev_gui.same_line();

		dev_gui.add_text(m_root_path);

		dev_gui.same_line();

		if (dev_gui.draw_button(m_browse_button) == true)
		{
			Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
			m_file_browser.open_at(file_system.get_absolute_path(Vadon::Core::FileSystemPath{ .root_directory = file_system.get_default_root(), .path = "" }));
		}

		const bool input_valid = has_valid_input();
		if (input_valid == false)
		{
			dev_gui.begin_disabled();
		}
		if (dev_gui.draw_button(m_create_button) == true)
		{
			if (validate() == true)
			{
				result = Result::ACCEPTED;
				close();
			}
			else
			{
				// TODO: show error!
			}
		}
		if (input_valid == false)
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

	void NewProjectDialog::on_open()
	{
		m_input_name.input.clear();
		m_root_path.clear();
	}

	NewProjectDialog::NewProjectDialog(Core::Editor& editor)
		: Dialog("Create Project")
		, m_editor(editor)
		, m_file_browser("Select Project Root Directory")
	{
		m_input_name.label = "Name";
		m_browse_button.label = "Browse";

		m_create_button.label = "Create";
		m_cancel_button.label = "Cancel";
	}

	bool NewProjectDialog::has_valid_input() const
	{
		if (m_input_name.input.empty() == true)
		{
			return false;
		}

		if (m_root_path.empty() == true)
		{
			return false;
		}

		return true;
	}
	
	bool NewProjectDialog::validate() const
	{
		// TODO: check if project conflicts with anything in the provided location!
		return true;
	}

	ProjectLauncher::ProjectLauncher(Core::Editor& editor)
		: m_editor(editor)
		, m_new_project_dialog(editor)
		, m_import_project_dialog("Import Project")
	{
		m_window.title = "Project Launcher";

		m_new_project_button.label = "Create new project";
		m_import_project_button.label = "Import project";
		m_open_project_button.label = "Open project";

		m_project_list.label = "#project_list";
	}

	void ProjectLauncher::initialize()
	{
		m_window.open = true;

		m_project_list.deselect();
		m_project_list.items.clear();

		Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();
		const Core::ProjectInfoList& available_projects = project_manager.get_project_cache();

		m_project_list.items.reserve(available_projects.size());

		for (const Core::ProjectInfo& current_project : available_projects)
		{
			m_project_list.items.push_back(std::format("{} ({})", current_project.name, current_project.root_path));
		}
	}

	void ProjectLauncher::draw(UI::Developer::GUISystem& dev_gui)
	{
		if (dev_gui.begin_window(m_window) == true)
		{
			Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();
			if (m_new_project_dialog.draw(dev_gui) == UI::Developer::Dialog::Result::ACCEPTED)
			{
				if (project_manager.create_project(m_new_project_dialog.m_input_name.input, m_new_project_dialog.m_root_path) == false)
				{
					// TODO: show error dialog!
				}
			}

			if (m_import_project_dialog.draw(dev_gui) == UI::Developer::Dialog::Result::ACCEPTED)
			{
				// TODO: import the project!
				// TODO: perform this asynchronously?
				if (project_manager.open_project(m_import_project_dialog.get_selected_file().path) == false)
				{
					// TODO: show error dialog!
				}
			}

			if (dev_gui.draw_button(m_new_project_button) == true)
			{
				m_new_project_dialog.open();
			}

			if (dev_gui.draw_button(m_import_project_button) == true)
			{
				Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
				m_import_project_dialog.open_at(file_system.get_absolute_path(Vadon::Core::FileSystemPath{ .root_directory = file_system.get_default_root(), .path = "" }));
			}

			m_project_list.size = dev_gui.get_available_content_region();
			const VadonApp::UI::Developer::GUIStyle gui_style = dev_gui.get_style();

			m_project_list.size.y -= dev_gui.calculate_text_size(m_open_project_button.label).y + gui_style.frame_padding.y * 2 + 5.0f;

			dev_gui.draw_list_box(m_project_list);

			if (m_project_list.has_valid_selection() == false)
			{
				dev_gui.begin_disabled();
			}

			if (dev_gui.draw_button(m_open_project_button) == true)
			{
				const Core::ProjectInfoList& project_cache = project_manager.get_project_cache();

				const Core::ProjectInfo& selected_project = project_cache[m_project_list.selected_item];

				// TODO: perform this asynchronously?
				if (project_manager.open_project(selected_project.root_path) == false)
				{
					// TODO: show error dialog!
				}
			}

			if (m_project_list.has_valid_selection() == false)
			{
				dev_gui.end_disabled();
			}
		}
		dev_gui.end_window();
	}

	ProjectPropertiesDialog::Result ProjectPropertiesDialog::internal_draw(UI::Developer::GUISystem& dev_gui)
	{
		Result result = Result::NONE;

		Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();
		const Core::Project& active_project = project_manager.get_active_project();

		// TODO: draw other attributes?
		dev_gui.add_text(active_project.info.name);
		dev_gui.add_separator();
		dev_gui.add_text("Properties");

		for (auto& current_property : m_property_editors)
		{
			if (current_property->render(dev_gui) == true)
			{
				bool added = false;
				const Vadon::Utilities::Property& property_data = current_property->get_property();
				for (Vadon::Utilities::Property& edited_property : m_edited_properties)
				{
					if (edited_property.name == property_data.name)
					{
						edited_property.value = current_property->get_property().value;
						added = true;
						break;
					}
				}
				if (added == false)
				{
					m_edited_properties.push_back(current_property->get_property());
				}
				current_property->value_updated();
			}
		}

		if (m_edited_properties.empty() == true)
		{
			dev_gui.begin_disabled();
		}
		if (dev_gui.draw_button(m_ok_button) == true)
		{
			result = Result::ACCEPTED;
			close();
		}
		if (m_edited_properties.empty() == true)
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

	void ProjectPropertiesDialog::on_open()
	{
		m_property_editors.clear();
		m_edited_properties.clear();

		Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();
		const Core::Project& active_project = project_manager.get_active_project();

		for (const Vadon::Utilities::Property& current_property : active_project.info.custom_properties)
		{
			m_property_editors.emplace_back(PropertyEditor::create_property_editor(m_editor, current_property));
		}
	}

	ProjectPropertiesDialog::ProjectPropertiesDialog(Core::Editor& editor)
		: Dialog("Edit Project Properties")
		, m_editor(editor)
	{
		m_ok_button.label = "Ok";
		m_cancel_button.label = "Cancel";
	}
}