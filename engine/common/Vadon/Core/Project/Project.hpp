#ifndef VADON_CORE_PROJECT_PROJECT_HPP
#define VADON_CORE_PROJECT_PROJECT_HPP
#include <Vadon/Scene/Resource/Resource.hpp>
namespace Vadon::Core
{
	class EngineCoreInterface;

	struct Project
	{
		constexpr static const char c_project_file_name[] = "project.vdpr";

		std::string name;
		std::string root_path;
		Vadon::Scene::ResourceID startup_scene;
		// TODO: any other settings?

		// FIXME: use properties?
		VADONCOMMON_API static bool is_valid_project_path(std::string_view path);

		VADONCOMMON_API bool save_project_file(EngineCoreInterface& engine_core);
		VADONCOMMON_API bool load_project_file(EngineCoreInterface& engine_core, std::string_view path);
	};

	using ProjectList = std::vector<Project>;
}
#endif