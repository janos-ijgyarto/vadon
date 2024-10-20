#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Private/Platform/Dummy/PlatformInterface.hpp>

#include <VadonApp/Private/Core/Application.hpp>

namespace VadonApp::Private::Platform::Dummy
{
	PlatformInterface::PlatformInterface(VadonApp::Core::Application& application)
		: Platform::PlatformInterface(application)
	{

	}

	void PlatformInterface::dispatch_events()
	{
		// TODO!!!
	}

	void PlatformInterface::register_event_callback(EventCallback /*callback*/)
	{
		// TODO
	}

	bool PlatformInterface::internal_initialize()
	{
		// TODO!!!
		m_main_window_info.window = m_application.get_config().platform_config.main_window_info;

		log_message("Dummy GUI initialized successfully!\n");
		return true;
	}

	void PlatformInterface::internal_shutdown()
	{
		log_message("Dummy GUI shut down successfully.\n");
	}
}