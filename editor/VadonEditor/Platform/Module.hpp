#ifndef VADONEDITOR_PLATFORM_MODULE_HPP
#define VADONEDITOR_PLATFORM_MODULE_HPP
#include <VadonEditor/Core/System/System.hpp>
#include <VadonEditor/Core/Module.hpp>
namespace VadonEditor::Platform
{
	class PlatformModule : public Core::SystemModule<Core::SystemModuleList, PlatformModule, class PlatformInterface>
	{
	};

	template<typename SysImpl>
	using PlatformSystem = Core::System<PlatformModule, SysImpl>;
}
#endif