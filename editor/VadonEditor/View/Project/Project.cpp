#include <VadonEditor/View/Project/Project.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <format>

namespace VadonEditor::View
{
	UI::Developer::Dialog::Result NewProjectDialog::internal_draw(UI::Developer::GUISystem& dev_gui)
	{
		Result result = Result::NONE;

		dev_gui.draw_input_text(m_input_name);
		dev_gui.draw_input_text(m_input_path); // FIXME: implement a file browser widget!

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
		m_input_path.input.clear();
	}

	NewProjectDialog::NewProjectDialog(Core::Editor& editor)
		: Dialog("Create Project")
		, m_editor(editor)
	{
		m_input_name.label = "Name";
		m_input_path.label = "Path";

		m_create_button.label = "Create";
		m_cancel_button.label = "Cancel";
	}

	bool NewProjectDialog::has_valid_input() const
	{
		if (m_input_name.input.empty() == true)
		{
			return false;
		}

		if (m_input_path.input.empty() == true)
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

		m_project_list.label = "Previous projects";
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
				if (project_manager.create_project(m_new_project_dialog.m_input_name.input, m_new_project_dialog.m_input_path.input) == false)
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
				m_import_project_dialog.open();
			}

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
}