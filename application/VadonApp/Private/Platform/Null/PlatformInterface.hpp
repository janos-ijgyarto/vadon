#ifndef VADONAPP_PRIVATE_PLATFORM_NULL_PLATFORMINTERFACE_HPP
#define VADONAPP_PRIVATE_PLATFORM_NULL_PLATFORMINTERFACE_HPP
#include <VadonApp/Private/Platform/PlatformInterface.hpp>
namespace VadonApp::Private::Platform::Null
{
	class PlatformInterface final : public VadonApp::Private::Platform::PlatformInterface
	{
	public:
		PlatformInterface(VadonApp::Core::Application& application);

		VadonApp::Platform::WindowHandle create_window(const VadonApp::Platform::WindowInfo& /*window_info*/) override { return VadonApp::Platform::WindowHandle(); }
		VadonApp::Platform::WindowHandle find_window(VadonApp::Platform::WindowID /*window_id*/) const override { return VadonApp::Platform::WindowHandle(); }
		bool is_window_valid(VadonApp::Platform::WindowHandle /*window_handle*/) const override { return false; }

		VadonApp::Platform::WindowID get_window_id(VadonApp::Platform::WindowHandle /*window_handle*/) const override { return 0; }

		std::string get_window_title(VadonApp::Platform::WindowHandle /*window_handle*/) const override { return ""; }
		void set_window_title(VadonApp::Platform::WindowHandle /*window_handle*/, std::string_view /*title*/) override {}

		Vadon::Math::Vector2i get_window_position(VadonApp::Platform::WindowHandle /*window_handle*/) const override { return Vadon::Math::Vector2i{}; }
		void set_window_position(VadonApp::Platform::WindowHandle /*window_handle*/, const Vadon::Math::Vector2i /*position*/) override {}

		Vadon::Math::Vector2i get_window_size(VadonApp::Platform::WindowHandle /*window_handle*/) const override { return Vadon::Math::Vector2i{}; }
		void set_window_size(VadonApp::Platform::WindowHandle /*window_handle*/, const Vadon::Math::Vector2i /*size*/) override {}

		VadonApp::Platform::WindowFlags get_window_flags(VadonApp::Platform::WindowHandle /*window_handle*/) const override { return VadonApp::Platform::WindowFlags::NONE; }

		void toggle_window_borderless_fullscreen(VadonApp::Platform::WindowHandle /*window_handle*/) override {}

		VadonApp::Platform::PlatformWindowHandle get_platform_window_handle(VadonApp::Platform::WindowHandle /*window_handle*/) const override { return nullptr; }

		Vadon::Math::Vector2i get_window_drawable_size(VadonApp::Platform::WindowHandle /*window_handle*/) const { return Vadon::Math::Vector2i{}; }

		bool is_window_focused(VadonApp::Platform::WindowHandle /*window_handle*/) const override { return false; }

		void new_frame() override { m_events_polled = false; }
		const VadonApp::Platform::PlatformEventList& poll_events() override { return m_event_list; }

		VadonApp::Platform::FeatureFlags get_feature_flags() const override { return VadonApp::Platform::FeatureFlags::NONE; }

		uint64_t get_performance_frequency() const override { return 0; }
		uint64_t get_performance_counter() const override { return 0; }

		void show_cursor(bool /*show*/) override {}
		void set_cursor(VadonApp::Platform::Cursor /*cursor*/) override {}

		void capture_mouse(bool /*capture*/) override {}
		void warp_mouse(VadonApp::Platform::WindowHandle /*window_handle*/, const Vadon::Math::Vector2i& /*mouse_position*/) override {}
		VadonApp::Platform::MouseState get_mouse_state() const override { return VadonApp::Platform::MouseState(); }
		VadonApp::Platform::MouseState get_global_mouse_state() const override { return VadonApp::Platform::MouseState(); }

		void set_clipboard_text(const char* /*text*/) override {}
		const char* get_clipboard_text() override { return nullptr; }
	protected:
		bool internal_initialize() override;
		void internal_shutdown() override;
	};
}
#endif