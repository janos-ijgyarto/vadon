#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Private/Platform/PlatformInterface.hpp>

#ifdef VADON_PLATFORM_INTERFACE_SDL
#include <VadonApp/Private/Platform/SDL/PlatformInterface.hpp>
#endif
#include <VadonApp/Private/Platform/Null/PlatformInterface.hpp>

namespace VadonApp::Private::Platform
{
	PlatformInterface::Implementation PlatformInterface::get_interface(VadonApp::Core::Application& application)
	{
#ifdef VADON_PLATFORM_INTERFACE_SDL
		return std::make_unique<SDL::PlatformInterface>(application);
#else
		return get_null_interface(application);
#endif
	}

	PlatformInterface::Implementation PlatformInterface::get_null_interface(VadonApp::Core::Application& application)
	{
		return std::make_unique<Null::PlatformInterface>(application);
	}

	bool PlatformInterface::initialize()
	{
		if (internal_initialize() == false)
		{
			return false;
		}

		if (m_input_system.initialize() == false)
		{
			return false;
		}

		return true;
	}

	void PlatformInterface::shutdown()
	{
		m_input_system.shutdown();
		internal_shutdown();
	}

	PlatformInterface::PlatformInterface(VadonApp::Core::Application& application) 
		: VadonApp::Platform::PlatformInterface(application)
		, m_input_system(application)
	{}
}