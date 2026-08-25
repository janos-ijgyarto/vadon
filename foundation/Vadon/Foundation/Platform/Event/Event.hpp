#ifndef VADON_FOUNDATION_PLATFORM_EVENT_EVENT_HPP
#define VADON_FOUNDATION_PLATFORM_EVENT_EVENT_HPP
#include <Vadon/Foundation/Platform/Input/Keyboard.hpp>
#include <Vadon/Foundation/Platform/Input/Mouse.hpp>
#include <string>
namespace Vadon
{
	namespace Foundation
	{
		// FIXME: support all events (e.g based on SDL)
		enum class PlatformEventType
		{
			WINDOW,
			MOUSE_MOTION,
			MOUSE_BUTTON,
			MOUSE_WHEEL,
			KEYBOARD,
			TEXT_INPUT,
			QUIT
		};

		enum class PlatformWindowEventType
		{
			NONE,
			SHOWN,
			HIDDEN,
			EXPOSED,
			MOVED,
			RESIZED,
			SIZE_CHANGED,
			MINIMIZED,
			MAXIMIZED,
			RESTORED,
			ENTER,
			LEAVE,
			FOCUS_GAINED,
			FOCUS_LOST,
			CLOSE,
			DISPLAY_CHANGED
		};

		struct PlatformWindowEvent
		{
			PlatformWindowEventType type = PlatformWindowEventType::NONE;
			unsigned int window_id;
			int data1 = 0;
			int data2 = 0;
			// TODO: other data?
		};

		struct PlatformMouseMotionEvent
		{
			int position_x = 0;
			int position_y = 0;
			int relative_motion_x = 0;
			int relative_motion_y = 0;
			// TODO: other data?
		};

		struct PlatformMouseButtonEvent
		{
			PlatformMouseButton button = PlatformMouseButton::INVALID;
			bool down = false;
			// TODO: other data?
		};

		struct PlatformMouseWheelEvent
		{
			int x = 0;
			int y = 0;
			float precise_x = 0.0f;
			float precise_y = 0.0f;
		};

		struct PlatformKeyboardEvent
		{
			PlatformKeyCode key = PlatformKeyCode::UNKNOWN;
			PlatformKeyModifiers modifiers = PlatformKeyModifiers::NONE;
			int native_code = 0;
			int native_scancode = 0;
			bool down = false;
		};

		struct PlatformTextInputEvent
		{
			std::string text;
		};

		struct PlatformQuitEvent
		{
			// TODO: data?
		};
	}
}
#endif