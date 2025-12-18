#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Core/Logger.hpp>
#include <Vadon/Core/Project/Project.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <filesystem>
#include <format>

namespace
{
	void project_serialization_error_log()
	{
		Vadon::Core::Logger::log_error("Project: failed to serialize project data!\n");
	}
}

namespace Vadon::Core
{
	bool Project::is_valid_project_path(std::string_view path)
	{
		std::filesystem::path project_file_path(path);

		// TODO: allow relative path as well?
		if (project_file_path.is_relative() == true)
		{
			return false;
		}

		if (std::filesystem::is_regular_file(project_file_path) == false)
		{
			return false;
		}

		if (project_file_path.filename() != c_project_file_name)
		{
			return false;
		}

		return true;
	}

	bool Project::serialize_project_data(Vadon::Utilities::Serializer& serializer, Project& project_data)
	{
		using SerializerResult = Vadon::Utilities::Serializer::Result;

		if (serializer.initialize() == false)
		{
			Logger::log_error("Project: unable to initialize serializer for project file!\n");
			return false;
		}

		if (serializer.serialize("name", project_data.name) != SerializerResult::SUCCESSFUL)
		{
			project_serialization_error_log();
			return false;
		}

		// Serialize any custom data to a separate object
		if (serializer.serialize("custom_project_data", project_data.custom_data_id) != SerializerResult::SUCCESSFUL)
		{
			project_serialization_error_log();
			return false;
		}

		if (serializer.finalize() == false)
		{
			Logger::log_error("Project: failed to finalize serializer for project file!\n");
			return false;
		}
	
		return true;
	}
}