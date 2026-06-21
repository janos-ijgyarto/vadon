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

		if (serializer.initialize() == false)
		{
			Logger::log_error("Project: unable to initialize serializer for project file!\n");
			return false;
		}

		constexpr ::Vadon::Foundation::UUID c_name_uuid = Utilities::string_to_uuid(::Vadon::Foundation::ProjectInfoSchema::c_name_property.id);
		constexpr ::Vadon::Foundation::UUID c_custom_data_uuid = Utilities::string_to_uuid(::Vadon::Foundation::ProjectInfoSchema::c_custom_data_id_property.id);

		if (serializer.is_reading() == true)
		{
			const Vadon::Utilities::Serializer::KeyVector key_strings = serializer.get_keys();
			for (const std::string& current_key : key_strings)
			{
				const ::Vadon::Foundation::UUID current_uuid = Utilities::parse_labeled_uuid(current_key);
				if (current_uuid == c_name_uuid)
				{
					if (serializer.serialize(current_key, project_data.name) != SerializerResult::SUCCESSFUL)
					{
						project_serialization_error_log();
						return false;
					}
				}
				else if (current_uuid == c_custom_data_uuid)
				{
					if (serializer.serialize(current_key, project_data.custom_data_id) != SerializerResult::SUCCESSFUL)
					{
						project_serialization_error_log();
						return false;
					}
				}
			}
		}
		else
		{
			if (serializer.serialize(Utilities::serialize_labeled_uuid("name", c_name_uuid), project_data.name) != SerializerResult::SUCCESSFUL)
			{
				project_serialization_error_log();
				return false;
			}

			if (project_data.custom_data_id.is_valid() == true)
			{
				// Serialize any custom data to a separate object
				if (serializer.serialize(Utilities::serialize_labeled_uuid("custom_data_id", c_custom_data_uuid), project_data.custom_data_id) != SerializerResult::SUCCESSFUL)
				{
					project_serialization_error_log();
					return false;
				}
			}
		}

		if (serializer.finalize() == false)
		{
			Logger::log_error("Project: failed to finalize serializer for project file!\n");
			return false;
		}
	
		return true;
	}
}