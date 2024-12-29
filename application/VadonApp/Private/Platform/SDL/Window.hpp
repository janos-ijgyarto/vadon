#ifndef VADONAPP_PRIVATE_PLATFORM_SDL_WINDOW_HPP
#define VADONAPP_PRIVATE_PLATFORM_SDL_WINDOW_HPP
#include <VadonApp/Platform/Window/Window.hpp>
#include <SDL.h>
namespace VadonApp::Private::Platform::SDL
{
	using WindowFlags = VadonApp::Platform::WindowFlags;

	using WindowID = VadonApp::Platform::WindowID;

	using WindowInfo = VadonApp::Platform::WindowInfo;

	using WindowHandle = VadonApp::Platform::WindowHandle;

	using PlatformWindowHandle = VadonApp::Platform::PlatformWindowHandle;

	struct SDLWindow
	{
		SDL_Window* sdl_window = nullptr;
	};
}
#endif