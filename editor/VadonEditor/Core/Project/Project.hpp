#ifndef VADONEDITOR_CORE_PROJECT_PROJECT_HPP
#define VADONEDITOR_CORE_PROJECT_PROJECT_HPP
#include <Vadon/Core/Project/Project.hpp>
#include <Vadon/Core/File/RootDirectory.hpp>
namespace VadonEditor::Core
{
	struct Project
	{
		Vadon::Core::Project info;
		Vadon::Core::RootDirectoryHandle root_dir_handle;
	};

	using ProjectList = std::vector<Project>;
}
#endif