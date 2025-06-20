#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Private/Platform/Null/PlatformInterface.hpp>

#include <VadonApp/Private/Core/Application.hpp>

namespace VadonApp::Private::Platform::Null
{
	PlatformInterface::PlatformInterface(VadonApp::Core::Application& application)
		: Platform::PlatformInterface(application)
	{

	}

	bool PlatformInterface::internal_initialize()
	{
		log_message("Null platform interface initialized successfully!\n");
		return true;
	}

	void PlatformInterface::internal_shutdown()
	{
		log_message("Null platform interface shut down successfully.\n");
	}
}