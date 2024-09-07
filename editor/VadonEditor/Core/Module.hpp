#ifndef VADONEDITOR_CORE_MODULE_HPP
#define VADONEDITOR_CORE_MODULE_HPP
#include <VadonEditor/Core/System/System.hpp>
#include <VadonEditor/Core/System/Module.hpp>
namespace VadonEditor::Core
{
	class CoreModule : public Core::SystemModule<Core::SystemModuleList, CoreModule, class ProjectManager>
	{
	};

	template<typename SysImpl>
	using CoreSystem = Core::System<CoreModule, SysImpl>;
}
#endif