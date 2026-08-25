#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Model/Resource/Database.hpp>

#include <Vadon/Core/File/FileSystem.hpp>
#include <Vadon/Core/Project/Project.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <filesystem>
#include <format>

namespace
{
	constexpr static const char c_invalid_path_error[] = "Project manager: invalid project path \"{}\"!\n";
}

namespace VadonEditor::Core
{
	ProjectManager::ProjectManager()
	{

	}

	bool ProjectManager::load_project(Vadon::Core::EngineCoreInterface& engine_core, std::string_view root_path)
	{
		Vadon::Core::Logger::log_message(std::format("Project manager: loading project at path \"{}\"\n", root_path));

		// First validate the path
		std::filesystem::path fs_root_path(root_path);

		if (Vadon::Core::Project::is_valid_project_path(root_path) == false)
		{
			// Assume we gave a path to the folder with the project file
			if (std::filesystem::is_directory(fs_root_path) == true)
			{
				fs_root_path /= Vadon::Core::Project::c_project_file_name;
				if (Vadon::Core::Project::is_valid_project_path(fs_root_path.string()) == false)
				{
					Vadon::Core::Logger::log_error(std::format(c_invalid_path_error, root_path));
					return false;
				}
			}
			else
			{
				Vadon::Core::Logger::log_error(std::format(c_invalid_path_error, root_path));
				return false;
			}
		}

		const std::string project_file_path = fs_root_path.generic_string();
		Vadon::Core::Project& project_info = m_active_project;
		project_info.custom_data_resource_id.invalidate(); // Reset custom properties

		Vadon::Core::RawFileDataBuffer project_file_data;

		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();
		if (file_system.load_file(project_file_path, project_file_data) == false)
		{
			Vadon::Core::Logger::log_error("Project manager: unable to load file!\n");
			return false;
		}

		if (Model::ResourceDatabase::sanitize_editor_resource_file(project_file_data) == false)
		{
			Vadon::Core::Logger::log_error("Project manager: failed to process file!\n");
			return false;
		}

		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(project_file_data, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);

		if (serializer->initialize() == false)
		{
			Vadon::Core::Logger::log_error("Project manager: failed to initialize serializer for project file!\n");
			return false;
		}

		if (Vadon::Core::Project::serialize_project_data(*serializer, project_info) == false)
		{
			Vadon::Core::Logger::log_error(std::format("Project manager: \"{}\" is not a valid project file!\n", project_file_path));
			return false;
		}

		if (serializer->finalize() == false)
		{
			Vadon::Core::Logger::log_error("Project manager: failed to finalize serializer for project file!\n");
			return false;
		}

		project_info.root_path = std::filesystem::path(project_file_path).parent_path().generic_string();

		Vadon::Core::Logger::log_message(std::format("Project manager: project \"{}\" loaded successfully!\n", project_info.name));

		return true;
	}
}