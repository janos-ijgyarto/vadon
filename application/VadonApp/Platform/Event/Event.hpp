#ifndef VADONAPP_PLATFORM_EVENT_EVENTHANDLER_HPP
#define VADONAPP_PLATFORM_EVENT_EVENTHANDLER_HPP
#include <VadonApp/Platform/Input/Keyboard.hpp>
#include <VadonApp/Platform/Input/Mouse.hpp>
#include <Vadon/Utilities/Math/Vector.hpp>
#include <variant>
namespace VadonApp::Platform
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

	enum class WindowEventType
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

	struct WindowEvent
	{
		WindowEventType type = WindowEventType::NONE;
		uint32_t window_id;
		int32_t data1 = 0;
		int32_t data2 = 0;
		// TODO: other data?
	};

	struct MouseMotionEvent
	{
		Vadon::Utilities::Vector2i position = { 0, 0 };
		Vadon::Utilities::Vector2i relative_motion = { 0, 0 };
		// TODO: other data?
	};

	struct MouseButtonEvent
	{
		MouseButton button = MouseButton::INVALID;
		bool down = false;
		// TODO: other data?
	};

	struct MouseWheelEvent
	{
		int32_t x = 0;
		int32_t y = 0;
		float precise_x = 0.0f;
		float precise_y = 0.0f;
	};

	struct KeyboardEvent
	{
		KeyCode key = KeyCode::UNKNOWN;
		KeyModifiers modifiers = KeyModifiers::NONE;
		int native_code = 0;
		int native_scancode = 0;
		bool down = false;
	};

	struct TextInputEvent
	{
		std::string text;
	};

	struct QuitEvent
	{
		// TODO: data?
	};
		
	using PlatformEvent = std::variant<WindowEvent, MouseMotionEvent, MouseButtonEvent, MouseWheelEvent, KeyboardEvent, TextInputEvent, QuitEvent>;
	using PlatformEventList = std::vector<PlatformEvent>;
}
#endif