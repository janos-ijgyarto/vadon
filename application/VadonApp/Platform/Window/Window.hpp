#ifndef VADONAPP_PLATFORM_WINDOW_WINDOW_HPP
#define VADONAPP_PLATFORM_WINDOW_WINDOW_HPP
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTarget.hpp>
#include <Vadon/Utilities/Enum/EnumClassBitFlag.hpp>
namespace VadonApp::Platform
{
	enum class WindowFlags
	{
		NONE = 0,
		FULLSCREEN = 1 << 0,
		BORDERLESS = 1 << 1,
		MINIMIZED = 1 << 2,
		MAXIMIZED = 1 << 3,
		VISIBLE = 1 << 4,
		HIDDEN = 1 << 5,
	};

	struct WindowInfo
	{
		std::string title;
		Vadon::Utilities::Vector2i position;
		Vadon::Utilities::Vector2i size;
		WindowFlags flags = WindowFlags::NONE;
	};

	struct RenderWindowInfo
	{
		WindowInfo window;
		Vadon::Render::WindowHandle render_handle;
		Vadon::Utilities::Vector2i drawable_size;
	};

	using WindowHandle = void*; // Platform-dependent window handle
}

namespace Vadon::Utilities
{
	template<>
	struct EnableEnumBitwiseOperators<VadonApp::Platform::WindowFlags> : public std::true_type
	{

	};
}
#endif