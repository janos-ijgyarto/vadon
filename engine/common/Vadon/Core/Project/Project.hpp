#ifndef VADON_CORE_PROJECT_PROJECT_HPP
#define VADON_CORE_PROJECT_PROJECT_HPP
#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>
namespace Vadon::Utilities
{
	class Serializer;
}
namespace Vadon::Core
{
	class EngineCoreInterface;

	struct Project
	{
		constexpr static const char c_project_file_name[] = "project.vdpr";

		std::string name;
		std::string root_path;
		Vadon::Utilities::PropertyList custom_properties;

		VADONCOMMON_API static bool is_valid_project_path(std::string_view path);

		VADONCOMMON_API static bool serialize_project_data(Vadon::Utilities::Serializer& serializer, Project& project_data);
	};

	using ProjectList = std::vector<Project>;
}
#endif