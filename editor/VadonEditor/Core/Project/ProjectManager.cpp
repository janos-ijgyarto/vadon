#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <filesystem>
#include <format>

namespace VadonEditor::Core
{
	bool ProjectManager::open_project(std::string_view /*root_path*/)
	{
		// Should only be used if we don't have an active project
		if (m_state == State::LAUNCHER)
		{
			log_error("Project manager: project already open!\n");
			return false;
		}

		// TODO: launch new instance of editor with the project path		
		m_state = State::PROJECT_OPENED;
		return true;
	}

	void ProjectManager::close_project()
	{
		if (m_state != State::PROJECT_ACTIVE)
		{
			log_error("Project manager: no active project!\n");
			return;
		}

		// TODO: launch instance with no args (to open as launcher)

		// Change state so Editor shuts down
		m_state = State::PROJECT_CLOSED;
	}

	ProjectManager::ProjectManager(Editor& editor)
		: System(editor)
		, m_state(State::LAUNCHER)
	{

	}

	bool ProjectManager::initialize()
	{
		// TODO: load project cache (if present)!

		if (open_startup_project() == false)
		{
			return false;
		}

		return true;
	}

	bool ProjectManager::open_startup_project()
	{
		// Check command line arg, load startup project if requested
		constexpr const char c_startup_project_arg[] = "project";
		if (m_editor.has_command_line_arg(c_startup_project_arg) == true)
		{
			if (load_project(m_editor.get_command_line_arg(c_startup_project_arg)) == false)
			{
				return false;
			}
		}

		return true;
	}

	bool ProjectManager::load_project(std::string_view root_path)
	{
		// First validate the path
		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		std::filesystem::path fs_root_path(root_path);

		if (Vadon::Core::Project::is_valid_project_path(root_path) == false)
		{
			constexpr static const char c_invalid_path_error[] = "Project manager: invalid project path \"{}\"!\n";

			// Assume we gave a path to the folder with the project file
			if (std::filesystem::is_directory(fs_root_path) == true)
			{
				fs_root_path /= Vadon::Core::Project::c_project_file_name;
				if (Vadon::Core::Project::is_valid_project_path(fs_root_path.string()) == false)
				{
					log_error(std::format(c_invalid_path_error, root_path));
					return false;
				}
			}
			else
			{
				log_error(std::format(c_invalid_path_error, root_path));
				return false;
			}
		}

		const std::string project_file_path = fs_root_path.string();
		Vadon::Core::Project& project_info = m_active_project.info;

		if (project_info.load_project_file(engine_core, project_file_path) == false)
		{
			Vadon::Core::Logger::log_error(std::format("Invalid project file at \"{}\"!\n", project_file_path));
			return false;
		}

		{
			Vadon::Core::RootDirectoryInfo project_dir_info;
			project_dir_info.path = project_info.root_path;

			// Add project root directory
			Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();
			m_active_project.root_dir_handle = file_system.add_root_directory(project_dir_info);
			if (m_active_project.root_dir_handle.is_valid() == false)
			{
				log_error("Project manager: failed to register project root directory!\n");
				return false;
			}
		}

		// Everything loaded successfully, set the editor state
		m_state = State::PROJECT_ACTIVE;
		return true;
	}
}