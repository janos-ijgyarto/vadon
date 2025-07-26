#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Core/Logger.hpp>
#include <Vadon/Core/Project/Project.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/PropertySerialization.hpp>

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

		// Serialize any custom properties to a separate object
		if (serializer.open_object("custom_properties") != SerializerResult::SUCCESSFUL)
		{
			project_serialization_error_log();
			return false;
		}

		if (serializer.is_reading() == true)
		{
			for (Vadon::Utilities::Property& current_property : project_data.custom_properties)
			{
				// Check if key is present (if not, assume we should just use default value)
				// FIXME: invert this to instead only process keys that are actually in the data?
				if (serializer.has_key(current_property.name) == false)
				{
					continue;
				}

				switch (current_property.data_type.type)
				{
				case Vadon::Utilities::ErasedDataType::TRIVIAL:
				{
					const SerializerResult result = Vadon::Utilities::process_trivial_property(serializer, current_property.name, current_property.value, current_property.data_type);
					if (result != SerializerResult::SUCCESSFUL)
					{
						project_serialization_error_log();
						return false;
					}
				}
				break;
				case Vadon::Utilities::ErasedDataType::RESOURCE_ID:
				{
					Vadon::Scene::ResourceID& resource_id = std::get<Vadon::Scene::ResourceID>(current_property.value);
					if (serializer.serialize(current_property.name, resource_id) != SerializerResult::SUCCESSFUL)
					{
						project_serialization_error_log();
						return false;
					}
				}
				break;
				}
			}
		}
		else
		{
			for (Vadon::Utilities::Property& current_property : project_data.custom_properties)
			{
				switch (current_property.data_type.type)
				{
				case Vadon::Utilities::ErasedDataType::TRIVIAL:
				{
					const SerializerResult result = Vadon::Utilities::process_trivial_property(serializer, current_property.name, current_property.value, current_property.data_type);
					if (result != SerializerResult::SUCCESSFUL)
					{
						project_serialization_error_log();
						return false;
					}
				}
				break;
				case Vadon::Utilities::ErasedDataType::RESOURCE_ID:
				{
					Vadon::Scene::ResourceID& resource_id = std::get<Vadon::Scene::ResourceID>(current_property.value);
					if (serializer.serialize(current_property.name, resource_id) != SerializerResult::SUCCESSFUL)
					{
						project_serialization_error_log();
						return false;
					}
				}
				break;
				}
			}
		}

		if (serializer.close_object() != SerializerResult::SUCCESSFUL)
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