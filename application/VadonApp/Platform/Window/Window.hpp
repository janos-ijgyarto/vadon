#ifndef VADONAPP_PLATFORM_WINDOW_WINDOW_HPP
#define VADONAPP_PLATFORM_WINDOW_WINDOW_HPP
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <Vadon/Utilities/Enum/EnumClassBitFlag.hpp>
#include <Vadon/Utilities/Math/Vector.hpp>
namespace VadonApp::Platform
{
	enum class WindowFlags
	{
		NONE = 0,
		FULLSCREEN = 1 << 0,
		SHOWN = 1 << 1,
		HIDDEN = 1 << 2,
		BORDERLESS = 1 << 3,
		RESIZABLE = 1 << 4,
		MINIMIZED = 1 << 5,
		MAXIMIZED = 1 << 6,
		MOUSE_GRABBED = 1 << 7,
		INPUT_FOCUS = 1 << 8,
		MOUSE_FOCUS = 1 << 9,
		MOUSE_CAPTURE = 1 << 10,
		ALWAYS_ON_TOP = 1 << 11,
		SKIP_TASKBAR = 1 << 12,
		KEYBOARD_GRABBED = 1 << 13
	};

	using WindowID = uint32_t;

	struct WindowInfo
	{
		std::string title;
		Vadon::Utilities::Vector2i position;
		Vadon::Utilities::Vector2i size;
		WindowFlags flags = WindowFlags::NONE;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Window, WindowHandle);

	using PlatformWindowHandle = void*; // Platform-dependent window handle
}

namespace Vadon::Utilities
{
	template<>
	struct EnableEnumBitwiseOperators<VadonApp::Platform::WindowFlags> : public std::true_type
	{

	};
}
#endif