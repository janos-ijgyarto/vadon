#ifndef VADONAPP_PLATFORM_PLATFORMMODULE_HPP
#define VADONAPP_PLATFORM_PLATFORMMODULE_HPP
#include <VadonApp/Core/System.hpp>
#include <VadonApp/Core/SystemModule.hpp>
namespace VadonApp::Platform
{
	class PlatformModule : public Core::SystemModule<Core::SystemModuleList, PlatformModule, class PlatformInterface>
	{
	};

	template<typename SysImpl>
	using PlatformSystem = Core::System<PlatformModule, SysImpl>;
}
#endif