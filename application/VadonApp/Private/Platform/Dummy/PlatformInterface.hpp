#ifndef VADONAPP_PRIVATE_PLATFORM_DUMMY_PLATFORMINTERFACE_HPP
#define VADONAPP_PRIVATE_PLATFORM_DUMMY_PLATFORMINTERFACE_HPP
#include <VadonApp/Private/Platform/PlatformInterface.hpp>
namespace VadonApp::Private::Platform::Dummy
{
	class PlatformInterface final : public VadonApp::Private::Platform::PlatformInterface
	{
	public:
		PlatformInterface(VadonApp::Core::Application& application);

		VadonApp::Platform::PlatformEventList read_events() override;

		VadonApp::Platform::RenderWindowInfo get_window_info() const override { return m_main_window_info; }
		VadonApp::Platform::WindowHandle get_window_handle() const override { return nullptr; }

		void move_window(const Vadon::Utilities::Vector2i& /*position*/) override {}
		void resize_window(const Vadon::Utilities::Vector2i& /*size*/) override {}

		bool is_window_focused() const override { return false; }

		VadonApp::Platform::FeatureFlags get_feature_flags() const override { return VadonApp::Platform::FeatureFlags::NONE; }

		uint64_t get_performance_frequency() const override { return 0; }
		uint64_t get_performance_counter() const override { return 0; }

		void show_cursor(bool /*show*/) override {}
		void set_cursor(VadonApp::Platform::Cursor /*cursor*/) override {}

		void capture_mouse(bool /*capture*/) override {}
		void warp_mouse(const Vadon::Utilities::Vector2i& /*mouse_position*/) override {}
		Vadon::Utilities::Vector2i get_mouse_position() const override { return Vadon::Utilities::Vector2i(); }

		void set_clipboard_text(const char* /*text*/) override {}
		const char* get_clipboard_text() override { return nullptr; }
	protected:
		bool initialize_internal() override;
		void shutdown_internal() override;
	private:
		VadonApp::Platform::RenderWindowInfo m_main_window_info;
	};
}
#endif