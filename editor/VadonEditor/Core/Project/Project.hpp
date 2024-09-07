#ifndef VADONEDITOR_CORE_PROJECT_PROJECT_HPP
#define VADONEDITOR_CORE_PROJECT_PROJECT_HPP
#include <Vadon/Core/Project/Project.hpp>
#include <Vadon/Core/File/RootDirectory.hpp>
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
		Vadon::Core::RootDirectoryHandle root_dir_handle;
	};
}
#endif