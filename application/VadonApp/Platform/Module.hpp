#ifndef VADONAPP_PLATFORM_MODULE_HPP
#define VADONAPP_PLATFORM_MODULE_HPP
#include <VadonApp/Core/System.hpp>
#include <VadonApp/Core/SystemModule.hpp>
namespace VadonApp::Platform
{
	class PlatformModule : public Core::SystemModule<Core::SystemModuleList, PlatformModule, class InputSystem, class PlatformInterface>
	{
	};

	template<typename SysImpl>
	using PlatformSystem = Core::System<PlatformModule, SysImpl>;
}
#endif