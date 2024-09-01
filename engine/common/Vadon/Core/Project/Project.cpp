#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Core/Project/Project.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <filesystem>

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

	bool Project::save_project_file(EngineCoreInterface& /*engine_core*/, std::string_view /*path*/)
	{
		// TODO!
		return false;
	}

	bool Project::load_project_file(EngineCoreInterface& engine_core, std::string_view path)
	{
		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();

		Vadon::Core::FileSystem::RawFileDataBuffer project_file_buffer;
		file_system.load_file(Vadon::Core::FileSystem::Path{ .path = path }, project_file_buffer);

		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(project_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);

		if (serializer->initialize() == false)
		{
			return false;
		}

		if (serializer->serialize("name", name) == false)
		{
			// TODO: notify about missing attribute?
			return false;
		}
		startup_scene.serialize(*serializer, "startup_scene");

		// TODO: any other data?

		if (serializer->finalize() == false)
		{
			return false;
		}

		root_path = std::filesystem::path(path).parent_path().string();
		return true;
	}
}