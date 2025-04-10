#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Core/Project/Project.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/PropertySerialization.hpp>

#include <filesystem>
#include <format>

namespace Vadon::Core
{
	namespace
	{
		void project_serialization_error_log()
		{
			Logger::log_error("Project: failed to serialize project data!\n");
		}
	}

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

	bool Project::save_project_file(EngineCoreInterface& engine_core, Project& project_data)
	{
		// TODO: decouple serializer settings so we can make binary project files as well?
		Vadon::Core::FileSystem::RawFileDataBuffer project_file_buffer;
		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(project_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::WRITE);

		using SerializerResult = Vadon::Utilities::Serializer::Result;

		if (serializer->initialize() == false)
		{
			Logger::log_error("Project: unable to initialize serializer for project file!\n");
			return false;
		}

		if (serializer->serialize("name", project_data.name) != SerializerResult::SUCCESSFUL)
		{
			project_serialization_error_log();
			return false;
		}

		// Serialize any custom properties to a separate object
		if (serializer->open_object("custom_properties") != SerializerResult::SUCCESSFUL)
		{
			project_serialization_error_log();
			return false;
		}

		for (Vadon::Utilities::Property& current_property : project_data.custom_properties)
		{
			switch (current_property.data_type.type)
			{
			case Vadon::Utilities::ErasedDataType::TRIVIAL:
			{
				const SerializerResult result = Vadon::Utilities::process_trivial_property(*serializer, current_property.name, current_property.value, current_property.data_type);
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
				if (serializer->serialize(current_property.name, resource_id) != SerializerResult::SUCCESSFUL)
				{
					project_serialization_error_log();
					return false;
				}
			}
			break;
			}
		}

		if (serializer->close_object() != SerializerResult::SUCCESSFUL)
		{
			project_serialization_error_log();
			return false;
		}

		if (serializer->finalize() == false)
		{
			Logger::log_error("Project: failed to finalize serializer for project file!\n");
			return false;
		}
	
		const std::string file_path = (std::filesystem::path(project_data.root_path) / c_project_file_name).string();

		Vadon::Core::Logger::log_message(std::format("Saving project file at \"{}\".\n", file_path));

		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();
		if (file_system.save_file(Vadon::Core::FileSystemPath{ .path = file_path }, project_file_buffer) == false)
		{
			Logger::log_error("Project: failed to save project to file!\n");
			return false;
		}

		return true;
	}

	bool Project::load_project_file(EngineCoreInterface& engine_core, std::string_view path, Project& project_data)
	{
		Vadon::Core::Logger::log_message(std::format("Loading project file at \"{}\".\n", path));

		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

		using SerializerResult = Vadon::Utilities::Serializer::Result;

		Vadon::Core::FileSystem::RawFileDataBuffer project_file_buffer;
		if (file_system.load_file(Vadon::Core::FileSystemPath{ .path = std::string(path) }, project_file_buffer) == false)
		{
			Logger::log_error("Project: failed to load project file!\n");
			return false;
		}

		// TODO: decouple serializer settings so we can make binary project files as well?
		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(project_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);

		if (serializer->initialize() == false)
		{
			Logger::log_error("Project: failed to initialize serializer while loading project!\n");
			return false;
		}

		if (serializer->serialize("name", project_data.name) != SerializerResult::SUCCESSFUL)
		{
			project_serialization_error_log();
			return false;
		}

		// Serialize any custom properties to a separate object
		if (serializer->open_object("custom_properties") != SerializerResult::SUCCESSFUL)
		{
			project_serialization_error_log();
			return false;
		}

		for (Vadon::Utilities::Property& current_property : project_data.custom_properties)
		{
			// Check if key is present (if not, assume we should just use default value)
			// FIXME: invert this to instead only process keys that are actually in the data?
			if (serializer->has_key(current_property.name) == false)
			{
				continue;
			}

			switch (current_property.data_type.type)
			{
			case Vadon::Utilities::ErasedDataType::TRIVIAL:
			{
				const SerializerResult result = Vadon::Utilities::process_trivial_property(*serializer, current_property.name, current_property.value, current_property.data_type);
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
				if (serializer->serialize(current_property.name, resource_id) != SerializerResult::SUCCESSFUL)
				{
					project_serialization_error_log();
					return false;
				}
			}
			break;
			}
		}

		if (serializer->close_object() != SerializerResult::SUCCESSFUL)
		{
			project_serialization_error_log();
			return false;
		}

		if (serializer->finalize() == false)
		{
			Logger::log_error("Project: failed to finalize serializer while loading project!\n");
			return false;
		}

		project_data.root_path = std::filesystem::path(path).parent_path().string();
		return true;
	}
}