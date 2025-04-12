#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <filesystem>
#include <format>

namespace
{
	constexpr static const char c_project_cache_file_name[] = "project_cache.vadon";
	constexpr static const char c_invalid_path_error[] = "Project manager: invalid project path \"{}\"!\n";
}

namespace VadonEditor::Core
{
	void ProjectManager::update_project_custom_properties(const Vadon::Utilities::PropertyList& properties)
	{
		for (const Vadon::Utilities::Property& current_property : properties)
		{
			for (Vadon::Utilities::Property& project_property : m_active_project.info.custom_properties)
			{
				if (project_property.name == current_property.name)
				{
					VADON_ASSERT(current_property.data_type == project_property.data_type, "Mismatch in property data type!");
					project_property.value = current_property.value;
					break;
				}
			}
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		if (Vadon::Core::Project::save_project_file(engine_core, m_active_project.info) == false)
		{
			log_error("Project manager: error saving project file!\n");
			VADON_ERROR("Failed to save project!");
			return;
		}

		for (const ProjectPropertiesCallback& current_callback : m_properties_callbacks)
		{
			current_callback(m_active_project);
		}
	}

	void ProjectManager::register_project_properties_callback(ProjectPropertiesCallback callback)
	{
		m_properties_callbacks.push_back(callback);
	}

	bool ProjectManager::create_project(std::string_view project_name, std::string_view root_path)
	{
		log_message(std::format("Project manager: creating project \"{}\" at path \"{}\"\n", project_name, root_path));

		// Should only be used if we don't have an active project
		if (m_state != State::LAUNCHER)
		{
			log_error("Project manager: project already open!\n");
			return false;
		}

		// Validate project parameters
		if (project_name.empty() == true)
		{
			log_error("Project manager: cannot create project without name!\n");
			return false;
		}

		if (root_path.empty() == true)
		{
			log_error("Project manager: cannot create project without root path!\n");
			return false;
		}

		// Validate path
		const std::filesystem::path fs_root_path(root_path);
		if (std::filesystem::is_directory(fs_root_path) == false)
		{
			log_error(std::format(c_invalid_path_error, root_path));
			return false;
		}
		else if (std::filesystem::is_empty(fs_root_path) == false)
		{
			log_error("Project manager: project path not empty!\n");
			return false;
		}

		// Create project file
		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Core::Project new_project_info{ .name = std::string(project_name), .root_path = std::string(root_path), .custom_properties = m_custom_properties };

		if (Vadon::Core::Project::save_project_file(engine_core, new_project_info) == false)
		{
			log_error("Project manager: error saving project file!\n");
			return false;
		}

		log_message("Project manager: new project created successfully!\n");

		// Successfully saved project file, now we load it (to reuse the same logic)
		return load_project(root_path);
	}

	bool ProjectManager::open_project(std::string_view root_path)
	{
		// Should only be used if we don't have an active project
		if (m_state != State::LAUNCHER)
		{
			log_error("Project manager: project already open!\n");
			return false;
		}

		log_message(std::format("Project manager: project opening requested at path \"{}\"\n", root_path));

		if (load_project(root_path) == false)
		{
			log_error("Project manager: invalid path!\n");
			return false;
		}

		return true;
	}

	void ProjectManager::close_project()
	{
		if (m_state != State::PROJECT_OPEN)
		{
			log_error("Project manager: no active project!\n");
			return;
		}

		// TODO: launch new instance of editor with no args (to open as launcher)

		// Change state so Editor shuts down
		m_state = State::PROJECT_CLOSED;
	}

	ProjectManager::ProjectManager(Editor& editor)
		: System(editor)
		, m_asset_library(editor)
		, m_state(State::LAUNCHER)
	{

	}

	bool ProjectManager::initialize()
	{
		load_project_cache();

		if (open_startup_project() == false)
		{
			return false;
		}

		return true;
	}

	void ProjectManager::load_project_cache()
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

		const Vadon::Core::FileSystemPath project_path{ .path = c_project_cache_file_name };

		if (file_system.does_file_exist(project_path) == false)
		{
			// No project cache
			return;
		}

		Vadon::Core::FileSystem::RawFileDataBuffer project_cache_buffer;
		if (file_system.load_file(project_path, project_cache_buffer) == false)
		{
			log_error("Project manager: unable to load file!\n");
			return;
		}

		constexpr static const char c_cache_file_data_error[] = "Project manager: project cache file contains invalid data!\n";

		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(project_cache_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);
		if (serializer->initialize() == false)
		{
			log_error(c_cache_file_data_error);
			return;
		}

		// TODO: use the return value for more elaborate parsing and error reporting?
		using SerializerResult = Vadon::Utilities::Serializer::Result;
		if (serializer->open_array("projects") != SerializerResult::SUCCESSFUL)
		{
			log_error(c_cache_file_data_error);
			return;
		}

		const size_t project_count = serializer->get_array_size();
		m_project_cache.reserve(project_count);

		for (size_t current_project_index = 0; current_project_index < project_count; ++current_project_index)
		{
			if (serializer->open_object(current_project_index) != SerializerResult::SUCCESSFUL)
			{
				log_error(c_cache_file_data_error);
				return;
			}

			ProjectInfo current_project_info;
			if (serializer->serialize("name", current_project_info.name) != SerializerResult::SUCCESSFUL)
			{
				log_error(c_cache_file_data_error);
				return;
			}

			if (serializer->serialize("path", current_project_info.root_path) != SerializerResult::SUCCESSFUL)
			{
				log_error(c_cache_file_data_error);
				return;
			}

			if (serializer->close_object() != SerializerResult::SUCCESSFUL)
			{
				log_error(c_cache_file_data_error);
				return;
			}

			m_project_cache.push_back(current_project_info);
		}

		if (serializer->close_array() != SerializerResult::SUCCESSFUL)
		{
			log_error(c_cache_file_data_error);
			return;
		}

		if (serializer->finalize() == false)
		{
			log_error(c_cache_file_data_error);
			return;
		}
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
		log_message(std::format("Project manager: loading project at path \"{}\"\n", root_path));

		// First validate the path
		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		std::filesystem::path fs_root_path(root_path);

		if (Vadon::Core::Project::is_valid_project_path(root_path) == false)
		{
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
		project_info.custom_properties = m_custom_properties; // Reset custom properties

		if (Vadon::Core::Project::load_project_file(engine_core, project_file_path, project_info) == false)
		{
			log_error(std::format("Project manager: \"{}\" is not a valid project file!\n", project_file_path));
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

		log_message(std::format("Project manager: project \"{}\" loaded successfully!\n", project_info.name));

		// Everything loaded successfuly, add to project cache
		add_project_to_cache(Core::ProjectInfo{ .name = project_info.name, .root_path = project_info.root_path });

		m_asset_library.rebuild_asset_tree();

		// Set the editor state
		m_state = State::PROJECT_LOADED;
		return true;
	}

	void ProjectManager::add_project_to_cache(const ProjectInfo& project)
	{
		// Remove previous entry (if present)
		{
			auto current_proj_it = std::find_if(m_project_cache.begin(), m_project_cache.end(), [&project](const ProjectInfo& current)
				{
					return project.root_path == current.root_path;
				}
			);

			if (current_proj_it != m_project_cache.end())
			{
				m_project_cache.erase(current_proj_it);
			}
		}
		
		m_project_cache.push_back(project);

		// Save project cache file
		constexpr static const char c_cache_file_data_error[] = "Project manager: error saving project cache!\n";
		Vadon::Core::FileSystem::RawFileDataBuffer project_cache_buffer;
		{
			Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(project_cache_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::WRITE);

			if (serializer->initialize() == false)
			{
				log_error(c_cache_file_data_error);
				return;
			}

			// TODO: use the return value for more elaborate parsing and error reporting?
			using SerializerResult = Vadon::Utilities::Serializer::Result;
			if (serializer->open_array("projects") != SerializerResult::SUCCESSFUL)
			{
				log_error(c_cache_file_data_error);
				return;
			}

			for (size_t current_project_index = 0; current_project_index < m_project_cache.size(); ++current_project_index)
			{
				if (serializer->open_object(current_project_index) != SerializerResult::SUCCESSFUL)
				{
					log_error(c_cache_file_data_error);
					return;
				}

				ProjectInfo& current_project_info = m_project_cache[current_project_index];
				if (serializer->serialize("name", current_project_info.name) != SerializerResult::SUCCESSFUL)
				{
					log_error(c_cache_file_data_error);
					return;
				}

				if (serializer->serialize("path", current_project_info.root_path) != SerializerResult::SUCCESSFUL)
				{
					log_error(c_cache_file_data_error);
					return;
				}

				if (serializer->close_object() != SerializerResult::SUCCESSFUL)
				{
					log_error(c_cache_file_data_error);
					return;
				}
			}

			if (serializer->close_array() != SerializerResult::SUCCESSFUL)
			{
				log_error(c_cache_file_data_error);
				return;
			}

			if (serializer->finalize() == false)
			{
				log_error(c_cache_file_data_error);
				return;
			}
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

		const Vadon::Core::FileSystemPath project_path{ .path = c_project_cache_file_name };
		if (file_system.save_file(project_path, project_cache_buffer) == false)
		{
			log_error("Project manager: unable to save file!\n");
			return;
		}
	}
}