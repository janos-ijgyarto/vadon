#ifndef VADON_FOUNDATION_PLATFORM_INPUT_MOUSE_HPP
#define VADON_FOUNDATION_PLATFORM_INPUT_MOUSE_HPP
namespace Vadon
{
	namespace Foundation
	{
		enum class PlatformMouseButton
		{
			LEFT,
			MIDDLE,
			RIGHT,
			X1,
			X2,
			BUTTON_COUNT,
			INVALID = -1
		};

		enum class PlatformCursor
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
}
#endif