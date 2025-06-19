#ifndef VADONAPP_CORE_CONFIGURATION_HPP
#define VADONAPP_CORE_CONFIGURATION_HPP
#include <VadonApp/Platform/Configuration.hpp>
#include <VadonApp/UI/Configuration.hpp>
namespace VadonApp::Core
{
	struct AppConfiguration
	{
		std::string program_name;
		// TODO!!!
	};

	struct Configuration
	{
		AppConfiguration app_config;
		VadonApp::Platform::Configuration platform_config;
		VadonApp::UI::Configuration ui_config;
	};
}
#endif