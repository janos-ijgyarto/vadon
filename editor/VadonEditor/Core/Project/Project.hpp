#ifndef VADONEDITOR_CORE_PROJECT_PROJECT_HPP
#define VADONEDITOR_CORE_PROJECT_PROJECT_HPP
#include <Vadon/Core/Project/Project.hpp>
namespace VadonEditor::Core
{
	struct ProjectInfo
	{
		std::string name;
		std::string root_path;
		// TODO: any other info?
	};

	using ProjectInfoList = std::vector<ProjectInfo>;

	struct Project
	{
		Vadon::Core::Project info;
		// TODO: anything else?
	};
}
#endif