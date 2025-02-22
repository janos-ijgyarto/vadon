#ifndef VADONAPP_PRIVATE_PLATFORM_SDL_PLATFORMINTERFACE_HPP
#define VADONAPP_PRIVATE_PLATFORM_SDL_PLATFORMINTERFACE_HPP
#include <VadonApp/Private/Platform/PlatformInterface.hpp>
#include <VadonApp/Private/Platform/Event/Event.hpp>

#include <VadonApp/Private/Platform/SDL/Window.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>

namespace VadonApp::Private::Platform::SDL
{
	using MouseState = VadonApp::Platform::MouseState;

	class PlatformInterface final : public VadonApp::Private::Platform::PlatformInterface
	{
	public:
		PlatformInterface(VadonApp::Core::Application& application);

		WindowHandle create_window(const WindowInfo& window_info) override;
		WindowHandle find_window(WindowID window_id) const override;
		bool is_window_valid(WindowHandle window_handle) const override { return m_window_pool.is_handle_valid(window_handle); }

		WindowID get_window_id(WindowHandle window_handle) const override;

		std::string get_window_title(WindowHandle window_handle) const override;
		void set_window_title(WindowHandle window_handle, std::string_view title) override;

		Vadon::Utilities::Vector2i get_window_position(WindowHandle window_handle) const override;
		void set_window_position(WindowHandle window_handle, const Vadon::Utilities::Vector2i position) override;

		Vadon::Utilities::Vector2i get_window_size(WindowHandle window_handle) const override;
		void set_window_size(WindowHandle window_handle, const Vadon::Utilities::Vector2i size) override;

		WindowFlags get_window_flags(WindowHandle window_handle) const override;

		void toggle_window_borderless_fullscreen(VadonApp::Platform::WindowHandle window_handle) override;

		PlatformWindowHandle get_platform_window_handle(WindowHandle window_handle) const override;

		Vadon::Utilities::Vector2i get_window_drawable_size(WindowHandle window_handle) const;

		bool is_window_focused(WindowHandle window_handle) const override;

		void poll_events() override;
		void register_event_callback(EventCallback callback) override;

		VadonApp::Platform::FeatureFlags get_feature_flags() const override;
		uint64_t get_performance_frequency() const override;
		uint64_t get_performance_counter() const override;

		void show_cursor(bool show) override;
		void set_cursor(VadonApp::Platform::Cursor cursor) override;

		void capture_mouse(bool capture) override;
		void warp_mouse(WindowHandle window_handle, const Vadon::Utilities::Vector2i& mouse_position) override;
		MouseState get_mouse_state() const override;
		MouseState get_global_mouse_state() const override;

		void set_clipboard_text(const char* text) override;
		const char* get_clipboard_text() override;
	protected:
		bool internal_initialize() override;
		void internal_shutdown() override;
	private:
		VadonApp::Platform::WindowEvent handle_window_event(const SDL_Event& sdl_event);

		void free_clipboard();

		char* m_clipboard;

		std::array<SDL_Cursor*, Vadon::Utilities::to_integral(VadonApp::Platform::Cursor::CURSOR_COUNT)> m_cursors;

		std::vector<EventCallback> m_event_callbacks;
		VadonApp::Platform::PlatformEventList m_platform_events;

		Vadon::Utilities::ObjectPool<VadonApp::Platform::Window, SDLWindow> m_window_pool;
		std::unordered_map<WindowID, WindowHandle> m_window_lookup;
	};
}
#endif