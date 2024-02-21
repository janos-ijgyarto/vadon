#ifndef VADONEDITOR_PLATFORM_EVENT_HPP
#define VADONEDITOR_PLATFORM_EVENT_HPP
#include <VadonApp/Platform/Event/Event.hpp>
namespace VadonEditor::Platform
{
	using PlatformEventType = VadonApp::Platform::PlatformEventType;

	using WindowEventType = VadonApp::Platform::WindowEventType;

	using KeyCode = VadonApp::Platform::KeyCode;
	using MouseButton = VadonApp::Platform::MouseButton;

	using WindowEvent = VadonApp::Platform::WindowEvent;
	using MouseMotionEvent = VadonApp::Platform::MouseMotionEvent;
	using MouseButtonEvent = VadonApp::Platform::MouseButtonEvent;
	using MouseWheelEvent = VadonApp::Platform::MouseWheelEvent;
	using KeyboardEvent = VadonApp::Platform::KeyboardEvent;
	using TextInputEvent = VadonApp::Platform::TextInputEvent;
	using QuitEvent = VadonApp::Platform::QuitEvent;

	using PlatformEvent = VadonApp::Platform::PlatformEvent;
	using PlatformEventList = VadonApp::Platform::PlatformEventList;
}
#endif