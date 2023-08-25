#ifndef VADONAPP_PLATFORM_CONFIGURATION_HPP
#define VADONAPP_PLATFORM_CONFIGURATION_HPP
#include <VadonApp/Platform/Window/Window.hpp>
namespace VadonApp::Platform
{
	struct Configuration
	{
		bool enable = true;
		WindowInfo main_window_info;
	};
}
#endif