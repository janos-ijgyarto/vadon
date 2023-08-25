#ifndef VADONAPP_PLATFORM_INPUT_MOUSE_HPP
#define VADONAPP_PLATFORM_INPUT_MOUSE_HPP
namespace VadonApp::Platform
{
	enum class MouseButton
	{
		LEFT,
		MIDDLE,
		RIGHT,
		X1,
		X2,
		INVALID = -1
	};

	enum class Cursor
	{
		ARROW,
		TEXT_INPUT,
		RESIZE_ALL,
		RESIZE_NS,
		RESIZE_EW,
		RESIZE_NESW,
		RESIZE_NWSE,
		HAND,
		NOT_ALLOWED,
		CURSOR_COUNT
	};
}
#endif