#ifndef VADONAPP_PLATFORM_PLATFORMINTERFACE_HPP
#define VADONAPP_PLATFORM_PLATFORMINTERFACE_HPP
#include <VadonApp/Platform/Module.hpp>

#include <VadonApp/Platform/Event/Event.hpp>
#include <VadonApp/Platform/Window/Window.hpp>

#include <Vadon/Utilities/Enum/EnumClassBitFlag.hpp>
namespace VadonApp::Platform
{
	// FIXME: more fitting name?
	// The features are based on metadata required by ImGui (might not be relevant for others?)
	enum class FeatureFlags
	{
		NONE = 0,
		MOUSE_GLOBAL_STATE = 1 << 0
	};

	struct MouseState
	{
		Vadon::Utilities::Vector2i position = { 0, 0 };
		// TODO: use enum for button state!
		uint32_t buttons = 0;
	};

	class PlatformInterface : public PlatformSystem<PlatformInterface>
	{
	public:
		using EventCallback = std::function<void(const PlatformEventList&)>;

		virtual ~PlatformInterface() {}

		virtual WindowHandle create_window(const WindowInfo& window_info) = 0;
		virtual WindowHandle find_window(WindowID window_id) const = 0;
		virtual bool is_window_valid(WindowHandle window_handle) const = 0;
		// TODO: remove window?

		virtual WindowID get_window_id(WindowHandle window_handle) const = 0;
		
		virtual std::string get_window_title(WindowHandle window_handle) const = 0;
		virtual void set_window_title(WindowHandle window_handle, std::string_view title) = 0;

		virtual Vadon::Utilities::Vector2i get_window_position(WindowHandle window_handle) const = 0;
		virtual void set_window_position(WindowHandle window_handle, const Vadon::Utilities::Vector2i position) = 0;

		virtual Vadon::Utilities::Vector2i get_window_size(WindowHandle window_handle) const = 0;
		virtual void set_window_size(WindowHandle window_handle, const Vadon::Utilities::Vector2i size) = 0;

		virtual WindowFlags get_window_flags(WindowHandle window_handle) const = 0;

		// FIXME: expose proper API for setting window states!
		virtual void toggle_window_borderless_fullscreen(WindowHandle window_handle) = 0;

		virtual PlatformWindowHandle get_platform_window_handle(WindowHandle window_handle) const = 0;

		virtual Vadon::Utilities::Vector2i get_window_drawable_size(WindowHandle window_handle) const = 0;

		virtual bool is_window_focused(WindowHandle window_handle) const = 0;

		// TODO: window show/hide/minimize/maximize/etc.!

		// NOTE: this will dispatch to all registered callbacks!
		virtual void poll_events() = 0;
		virtual void register_event_callback(EventCallback callback) = 0;

		virtual FeatureFlags get_feature_flags() const = 0;
		virtual uint64_t get_performance_frequency() const = 0;
		virtual uint64_t get_performance_counter() const = 0;

		virtual void show_cursor(bool show) = 0;
		virtual void set_cursor(Cursor cursor) = 0;

		virtual void capture_mouse(bool capture) = 0;
		virtual void warp_mouse(WindowHandle window_handle, const Vadon::Utilities::Vector2i& mouse_position) = 0;
		virtual MouseState get_mouse_state() const = 0;
		virtual MouseState get_global_mouse_state() const = 0;

		virtual void set_clipboard_text(const char* text) = 0;
		virtual const char* get_clipboard_text() = 0;
	protected:
		PlatformInterface(Core::Application& application)
			: System(application)
		{}
	};
}

namespace Vadon::Utilities
{
	template <>
	struct EnableEnumBitwiseOperators<VadonApp::Platform::FeatureFlags> : public std::true_type
	{};
}
#endif