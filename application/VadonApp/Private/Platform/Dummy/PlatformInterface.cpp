#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Private/Platform/Dummy/PlatformInterface.hpp>

#include <VadonApp/Private/Core/Application.hpp>

namespace VadonApp::Private::Platform::Dummy
{
	PlatformInterface::PlatformInterface(VadonApp::Core::Application& application)
		: Platform::PlatformInterface(application)
	{

	}

	VadonApp::Platform::PlatformEventList PlatformInterface::read_events()
	{
		// TODO!!!
		return VadonApp::Platform::PlatformEventList();
	}

	bool PlatformInterface::initialize()
	{
		// TODO!!!
		m_main_window_info.window = m_application.get_config().platform_config.main_window_info;

		log("Dummy GUI initialized successfully!\n");
		return true;
	}

	void PlatformInterface::shutdown()
	{
		log("Dummy GUI shut down successfully.\n");
	}
}