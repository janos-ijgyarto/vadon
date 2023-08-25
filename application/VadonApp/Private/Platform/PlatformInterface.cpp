#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Private/Platform/PlatformInterface.hpp>

#ifdef VADON_PLATFORM_INTERFACE_SDL
#include <VadonApp/Private/Platform/SDL/PlatformInterface.hpp>
#endif
#include <VadonApp/Private/Platform/Dummy/PlatformInterface.hpp>

namespace VadonApp::Private::Platform
{
	PlatformInterface::Implementation PlatformInterface::get_interface(VadonApp::Core::Application& application)
	{
#ifdef VADON_PLATFORM_INTERFACE_SDL
		return std::make_unique<SDL::PlatformInterface>(application);
#else
		return get_dummy_interface(application);
#endif
	}

	PlatformInterface::Implementation PlatformInterface::get_dummy_interface(VadonApp::Core::Application& application)
	{
		return std::make_unique<Dummy::PlatformInterface>(application);
	}
}