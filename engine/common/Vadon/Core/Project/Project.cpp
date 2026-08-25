#include <Vadon/Core/Logger.hpp>
#include <Vadon/Core/Project/Project.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>
#include <Vadon/Utilities/System/UUID/UUID.hpp>

#include <Vadon/Foundation/Project/Project.hpp>

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

		constexpr ::Vadon::Foundation::UUID c_name_uuid = Utilities::string_to_uuid(::Vadon::Foundation::ProjectInfoSchema::c_name_property.id);
		constexpr ::Vadon::Foundation::UUID c_custom_data_resource_uuid = Utilities::string_to_uuid(::Vadon::Foundation::ProjectInfoSchema::c_custom_data_resource_property.id);

		// Serialize project name
		if (serializer.serialize(c_name_uuid, project_data.name) != SerializerResult::SUCCESSFUL)
		{
			project_serialization_error_log();
			return false;
		}

		// Serialize UUID of custom data resource (project-dependent)
		const bool serialize_custom_data = (serializer.is_reading() && serializer.has_key(c_custom_data_resource_uuid)) || ((serializer.is_reading() == false) && (project_data.custom_data_resource_id.is_valid() == true));
		if(serialize_custom_data == true)
		{
			if (serializer.serialize(c_custom_data_resource_uuid, project_data.custom_data_resource_id) != SerializerResult::SUCCESSFUL)
			{
				project_serialization_error_log();
				return false;
			}
		}
	
		return true;
	}
}