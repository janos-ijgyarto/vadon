#ifndef VADONAPP_PLATFORM_EVENT_EVENTHANDLER_HPP
#define VADONAPP_PLATFORM_EVENT_EVENTHANDLER_HPP
#include <VadonApp/Utilities/Event/Event.hpp>
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
		MOVED,
		RESIZED,
		ENTER,
		LEAVE,
		FOCUS_GAINED,
		FOCUS_LOST
	};

	struct WindowEvent : public Utilities::Event<const WindowEvent&, bool>
	{
		WindowEventType type = WindowEventType::NONE;
		int32_t data1 = 0;
		int32_t data2 = 0;
		// TODO: other data?
	};

	struct MouseMotionEvent : public Utilities::Event<const MouseMotionEvent&, bool>
	{
		Vadon::Utilities::Vector2i position;
		// TODO: other data?
	};

	struct MouseButtonEvent : public Utilities::Event<const MouseButtonEvent&, bool>
	{
		MouseButton button = MouseButton::INVALID;
		bool down = false;
		// TODO: other data?
	};

	struct MouseWheelEvent : public Utilities::Event<const MouseWheelEvent&, bool>
	{
		int32_t x = 0;
		int32_t y = 0;
		float precise_x = 0.0f;
		float precise_y = 0.0f;
	};

	struct KeyboardEvent : public Utilities::Event<const KeyboardEvent&, bool>
	{
		KeyCode key = KeyCode::UNKNOWN;
		KeyModifiers modifiers = KeyModifiers::NONE;
		int native_code = 0;
		int native_scancode = 0;
		bool down = false;
	};

	struct TextInputEvent : public Utilities::Event<const TextInputEvent&, bool>
	{
		std::string text;
	};

	struct QuitEvent : public Utilities::Event<const QuitEvent&, bool>
	{
		// TODO: data?
	};

	using PlatformEventDispatcherPool = Utilities::EventDispatcherPool<WindowEvent::Handler, MouseMotionEvent::Handler, MouseButtonEvent::Handler, MouseWheelEvent::Handler, KeyboardEvent::Handler, TextInputEvent::Handler, QuitEvent::Handler>;
	
	using PlatformEvent = std::variant<WindowEvent, MouseMotionEvent, MouseButtonEvent, MouseWheelEvent, KeyboardEvent, TextInputEvent, QuitEvent>;
	using PlatformEventList = std::vector<PlatformEvent>;
}
#endif