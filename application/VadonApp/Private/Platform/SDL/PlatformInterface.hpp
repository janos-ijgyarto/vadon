#ifndef VADONAPP_PRIVATE_PLATFORM_SDL_PLATFORMINTERFACE_HPP
#define VADONAPP_PRIVATE_PLATFORM_SDL_PLATFORMINTERFACE_HPP
#include <VadonApp/Private/Platform/PlatformInterface.hpp>
#include <VadonApp/Private/Platform/Event/Event.hpp>

#include <Vadon/Utilities/Enum/EnumClass.hpp>

#include <SDL.h>

namespace VadonApp::Private::Platform::SDL
{
	class PlatformInterface final : public VadonApp::Private::Platform::PlatformInterface
	{
	public:
		PlatformInterface(VadonApp::Core::Application& application);

		VadonApp::Platform::PlatformEventList read_events() override;

		VadonApp::Platform::RenderWindowInfo get_window_info() const override;
		VadonApp::Platform::WindowHandle get_window_handle() const override;

		void move_window(const Vadon::Utilities::Vector2i& position) override;
		void resize_window(const Vadon::Utilities::Vector2i& size) override;

		bool is_window_focused() const override;

		VadonApp::Platform::FeatureFlags get_feature_flags() const override;
		uint64_t get_performance_frequency() const override;
		uint64_t get_performance_counter() const override;

		void show_cursor(bool show) override;
		void set_cursor(VadonApp::Platform::Cursor cursor) override;

		void capture_mouse(bool capture) override;
		void warp_mouse(const Vadon::Utilities::Vector2i& mouse_position) override;
		Vadon::Utilities::Vector2i get_mouse_position() const override;

		void set_clipboard_text(const char* text) override;
		const char* get_clipboard_text() override;
	protected:
		bool initialize_internal() override;
		void shutdown_internal() override;
	private:
		VadonApp::Platform::WindowEvent handle_window_event(const SDL_Event& sdl_event);
		void window_moved(const Vadon::Utilities::Vector2i& position);

		void cache_window_drawable_size();
		void free_clipboard();

		struct MainWindow
		{
			VadonApp::Platform::RenderWindowInfo render_window;
			SDL_Window* sdl_window = nullptr;
		};

		const char* working_dir = nullptr;
		MainWindow m_main_window;
		char* m_clipboard;

		std::array<SDL_Cursor*, Vadon::Utilities::to_integral(VadonApp::Platform::Cursor::CURSOR_COUNT)> m_cursors;
	};
}
#endif