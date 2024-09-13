#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Private/Platform/SDL/PlatformInterface.hpp>

#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>

#include <SDL_syswm.h>

// FIXME: defines taken from ImGui, should be investigated if it fits general usage
#if SDL_VERSION_ATLEAST(2,0,4) //&& !defined(__EMSCRIPTEN__) && !defined(__ANDROID__) && !(defined(__APPLE__) && TARGET_OS_IOS) && !defined(__amigaos4__)
#define SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE    1
#else
#define SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE    0
#endif

namespace VadonApp::Private::Platform::SDL
{
	namespace
	{
		// TODO: move to utility header?
		Vadon::Utilities::Vector2i get_sdl_window_position(SDL_Window* window)
		{
			int pos_x = 0, pos_y = 0;
			SDL_GetWindowPosition(window, &pos_x, &pos_y);

			return Vadon::Utilities::Vector2i(pos_x, pos_y);
		}

		Vadon::Utilities::Vector2i get_sdl_window_size(SDL_Window* window)
		{
			int width = 0, height = 0;
			SDL_GetWindowSize(window, &width, &height);

			return Vadon::Utilities::Vector2i(width, height);
		}

		constexpr VadonApp::Platform::WindowEventType convert_sdl_to_platform_window_event_type(SDL_WindowEventID event_id)
		{
			switch (event_id)
			{
			case SDL_WindowEventID::SDL_WINDOWEVENT_MOVED:
				return VadonApp::Platform::WindowEventType::MOVED;
			case SDL_WindowEventID::SDL_WINDOWEVENT_RESIZED:
			case SDL_WindowEventID::SDL_WINDOWEVENT_SIZE_CHANGED:
				return VadonApp::Platform::WindowEventType::RESIZED;
			case SDL_WindowEventID::SDL_WINDOWEVENT_ENTER:
				return VadonApp::Platform::WindowEventType::ENTER;
			case SDL_WindowEventID::SDL_WINDOWEVENT_LEAVE:
				return VadonApp::Platform::WindowEventType::LEAVE;
			case SDL_WindowEventID::SDL_WINDOWEVENT_FOCUS_GAINED:
				return VadonApp::Platform::WindowEventType::FOCUS_GAINED;
			case SDL_WindowEventID::SDL_WINDOWEVENT_FOCUS_LOST:
				return VadonApp::Platform::WindowEventType::FOCUS_LOST;
			}

			return VadonApp::Platform::WindowEventType::NONE;
		}

		constexpr SDL_SystemCursor get_sdl_cursor(VadonApp::Platform::Cursor cursor)
		{
			switch (cursor)
			{
			case VadonApp::Platform::Cursor::ARROW:
				return SDL_SYSTEM_CURSOR_ARROW;
			case VadonApp::Platform::Cursor::TEXT_INPUT:
				return SDL_SYSTEM_CURSOR_IBEAM;
			case VadonApp::Platform::Cursor::RESIZE_ALL:
				return SDL_SYSTEM_CURSOR_SIZEALL;
			case VadonApp::Platform::Cursor::RESIZE_NS:
				return SDL_SYSTEM_CURSOR_SIZENS;
			case VadonApp::Platform::Cursor::RESIZE_EW:
				return SDL_SYSTEM_CURSOR_SIZEWE;
			case VadonApp::Platform::Cursor::RESIZE_NESW:
				return SDL_SYSTEM_CURSOR_SIZENESW;
			case VadonApp::Platform::Cursor::RESIZE_NWSE:
				return SDL_SYSTEM_CURSOR_SIZENWSE;
			case VadonApp::Platform::Cursor::HAND:
				return SDL_SYSTEM_CURSOR_HAND;
			case VadonApp::Platform::Cursor::NOT_ALLOWED:
				return SDL_SYSTEM_CURSOR_NO;
			}

			return SDL_SYSTEM_CURSOR_ARROW;
		}

		constexpr VadonApp::Platform::KeyCode convert_sdl_to_platform_key_code(SDL_Keycode key_code)
		{
			using KeyCode = VadonApp::Platform::KeyCode;
			switch (key_code)
			{
			case SDLK_RETURN:
				return KeyCode::RETURN;
			case SDLK_ESCAPE:
				return KeyCode::ESCAPE;
			case SDLK_BACKSPACE:
				return KeyCode::BACKSPACE;
			case SDLK_SPACE:
				return KeyCode::SPACE;
			case SDLK_0:
				return KeyCode::KEY_0;
			case SDLK_1:
				return KeyCode::KEY_1;
			case SDLK_2:
				return KeyCode::KEY_2;
			case SDLK_3:
				return KeyCode::KEY_3;
			case SDLK_4:
				return KeyCode::KEY_4;
			case SDLK_5:
				return KeyCode::KEY_5;
			case SDLK_6:
				return KeyCode::KEY_6;
			case SDLK_7:
				return KeyCode::KEY_7;
			case SDLK_8:
				return KeyCode::KEY_8;
			case SDLK_9:
				return KeyCode::KEY_9;
			case SDLK_BACKQUOTE:
				return KeyCode::BACKQUOTE;
			case SDLK_a:
				return KeyCode::KEY_a;
			case SDLK_b:
				return KeyCode::KEY_b;
			case SDLK_c:
				return KeyCode::KEY_c;
			case SDLK_d:
				return KeyCode::KEY_d;
			case SDLK_e:
				return KeyCode::KEY_e;
			case SDLK_f:
				return KeyCode::KEY_f;
			case SDLK_g:
				return KeyCode::KEY_g;
			case SDLK_h:
				return KeyCode::KEY_h;
			case SDLK_i:
				return KeyCode::KEY_i;
			case SDLK_j:
				return KeyCode::KEY_j;
			case SDLK_k:
				return KeyCode::KEY_k;
			case SDLK_l:
				return KeyCode::KEY_l;
			case SDLK_m:
				return KeyCode::KEY_m;
			case SDLK_n:
				return KeyCode::KEY_n;
			case SDLK_o:
				return KeyCode::KEY_o;
			case SDLK_p:
				return KeyCode::KEY_p;
			case SDLK_q:
				return KeyCode::KEY_q;
			case SDLK_r:
				return KeyCode::KEY_r;
			case SDLK_s:
				return KeyCode::KEY_s;
			case SDLK_t:
				return KeyCode::KEY_t;
			case SDLK_u:
				return KeyCode::KEY_u;
			case SDLK_v:
				return KeyCode::KEY_v;
			case SDLK_w:
				return KeyCode::KEY_w;
			case SDLK_x:
				return KeyCode::KEY_x;
			case SDLK_y:
				return KeyCode::KEY_y;
			case SDLK_z:
				return KeyCode::KEY_z;
			case SDLK_INSERT:
				return KeyCode::INSERT;
			case SDLK_HOME:
				return KeyCode::HOME;
			case SDLK_PAGEUP:
				return KeyCode::PAGE_UP;
			case SDLK_DELETE:
				return KeyCode::DELETE_KEY;
			case SDLK_END:
				return KeyCode::END;
			case SDLK_PAGEDOWN:
				return KeyCode::PAGE_DOWN;
			case SDLK_RIGHT:
				return KeyCode::RIGHT;
			case SDLK_LEFT:
				return KeyCode::LEFT;
			case SDLK_DOWN:
				return KeyCode::DOWN;
			case SDLK_UP:
				return KeyCode::UP;
			case SDLK_LCTRL:
				return KeyCode::LEFT_CTRL;
			case SDLK_LSHIFT:
				return KeyCode::LEFT_SHIFT;
			}

			return KeyCode::UNKNOWN;
		}

		VadonApp::Platform::KeyModifiers convert_sdl_to_platform_key_modifiers(uint16_t modifiers)
		{
			VadonApp::Platform::KeyModifiers key_modifiers = VadonApp::Platform::KeyModifiers::NONE;

			// FIXME: implement more elegant iteration through flags
			if (modifiers & SDL_Keymod::KMOD_LSHIFT)
			{
				key_modifiers |= VadonApp::Platform::KeyModifiers::LEFT_SHIFT;
			}

			if (modifiers & SDL_Keymod::KMOD_RSHIFT)
			{
				key_modifiers |= VadonApp::Platform::KeyModifiers::RIGHT_SHIFT;
			}

			if (modifiers & SDL_Keymod::KMOD_LCTRL)
			{
				key_modifiers |= VadonApp::Platform::KeyModifiers::LEFT_CTRL;
			}

			if (modifiers & SDL_Keymod::KMOD_RCTRL)
			{
				key_modifiers |= VadonApp::Platform::KeyModifiers::RIGHT_CTRL;
			}

			return key_modifiers;
		}
	}

	PlatformInterface::PlatformInterface(VadonApp::Core::Application& application)
		: VadonApp::Private::Platform::PlatformInterface(application)
		, m_clipboard(nullptr)
	{}

	VadonApp::Platform::PlatformEventList PlatformInterface::read_events()
	{
		// Use static so we don't keep reallocating
		static VadonApp::Platform::PlatformEventList platform_events;
		platform_events.clear();

		SDL_Event sdl_event;
		while (SDL_PollEvent(&sdl_event) != 0)
		{
			switch (sdl_event.type)
			{
			case SDL_QUIT:
				platform_events.emplace_back(VadonApp::Platform::QuitEvent{});
				break;
			case SDL_WINDOWEVENT:
			{
				VadonApp::Platform::WindowEvent window_event = handle_window_event(sdl_event);
				platform_events.emplace_back(window_event);
			}
			break;
			case SDL_TEXTINPUT:
			{
				VadonApp::Platform::TextInputEvent text_input;
				text_input.text = sdl_event.text.text;

				platform_events.emplace_back(text_input);
			}
			break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			{
				VadonApp::Platform::KeyboardEvent keyboard_event;
				keyboard_event.key = convert_sdl_to_platform_key_code(sdl_event.key.keysym.sym);
				keyboard_event.modifiers = convert_sdl_to_platform_key_modifiers(sdl_event.key.keysym.mod);
				keyboard_event.native_code = sdl_event.key.keysym.sym;
				keyboard_event.native_scancode = sdl_event.key.keysym.scancode;
				keyboard_event.down = (sdl_event.type == SDL_KEYDOWN);

				platform_events.emplace_back(keyboard_event);
			}
			break;
			case SDL_MOUSEMOTION:
			{
				VadonApp::Platform::MouseMotionEvent motion_event;
				motion_event.position.x = sdl_event.motion.x;
				motion_event.position.y = sdl_event.motion.y;
				motion_event.relative_motion.x = sdl_event.motion.xrel;
				motion_event.relative_motion.y = sdl_event.motion.yrel;

				platform_events.emplace_back(motion_event);
			}
			break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			{
				VadonApp::Platform::MouseButtonEvent button_event;
				button_event.down = (sdl_event.type == SDL_MOUSEBUTTONDOWN);

				switch (sdl_event.button.button)
				{
				case SDL_BUTTON_LEFT:
					button_event.button = VadonApp::Platform::MouseButton::LEFT;
					break;
				case SDL_BUTTON_RIGHT:
					button_event.button = VadonApp::Platform::MouseButton::RIGHT;
					break;
				case SDL_BUTTON_MIDDLE:
					button_event.button = VadonApp::Platform::MouseButton::MIDDLE;
					break;
				case SDL_BUTTON_X1:
					button_event.button = VadonApp::Platform::MouseButton::X1;
					break;
				case SDL_BUTTON_X2:
					button_event.button = VadonApp::Platform::MouseButton::X2;
					break;
				}

				if (button_event.button == VadonApp::Platform::MouseButton::INVALID)
				{
					break;
				}

				platform_events.emplace_back(button_event);
			}
			break;
			case SDL_MOUSEWHEEL:
			{
				VadonApp::Platform::MouseWheelEvent wheel_event;
				wheel_event.x = sdl_event.wheel.x;
				wheel_event.y = sdl_event.wheel.y;
#if SDL_VERSION_ATLEAST(2,0,18)
				wheel_event.precise_x = sdl_event.wheel.preciseX;
				wheel_event.precise_y = sdl_event.wheel.preciseY;
#else
				wheel_event.precise_x = sdl_event.wheel.x;
				wheel_event.precise_y = sdl_event.wheel.y;
#endif

				platform_events.emplace_back(wheel_event);
			}
			break;
			}
		}

		return platform_events;
	}

	VadonApp::Platform::RenderWindowInfo PlatformInterface::get_window_info() const
	{
		// FIXME: need to make sure our flags are up to date!
		return m_main_window.render_window;
	}

	VadonApp::Platform::WindowHandle PlatformInterface::get_window_handle() const
	{
		// Get the HWND from the SDL window
		// FIXME: branch on OS!!!
		SDL_SysWMinfo sdl_wm_info;
		SDL_GetVersion(&sdl_wm_info.version);
		SDL_GetWindowWMInfo(m_main_window.sdl_window, &sdl_wm_info);

		return sdl_wm_info.info.win.window;
	}

	void PlatformInterface::move_window(const Vadon::Utilities::Vector2i& position)
	{
		SDL_SetWindowPosition(m_main_window.sdl_window, position.x, position.y);
		window_moved(position);
	}

	void PlatformInterface::resize_window(const Vadon::Utilities::Vector2i& size)
	{
		SDL_SetWindowSize(m_main_window.sdl_window, size.x, size.y);
	}

	bool PlatformInterface::is_window_focused() const
	{
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
		SDL_Window* focused_window = SDL_GetKeyboardFocus();
		return (m_main_window.sdl_window == focused_window);
#else
		return ((SDL_GetWindowFlags(m_main_window.sdl_window) & SDL_WINDOW_INPUT_FOCUS) != 0); // SDL 2.0.3 and non-windowed systems: single-viewport only
#endif
	}

	VadonApp::Platform::FeatureFlags PlatformInterface::get_feature_flags() const
	{
		VadonApp::Platform::FeatureFlags feature_flags = VadonApp::Platform::FeatureFlags::NONE;

		// Check and store if we are on a SDL backend that supports global mouse position
		// ("wayland" and "rpi" don't support it, but we chose to use a white-list instead of a black-list)
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
		const char* sdl_backend = SDL_GetCurrentVideoDriver();
		constexpr const char* global_mouse_whitelist[] = { "windows", "cocoa", "x11", "DIVE", "VMAN" };
		for (const char* current_entry : global_mouse_whitelist)
		{
			if (strncmp(sdl_backend, current_entry, strlen(current_entry)) == 0)
			{
				feature_flags |= VadonApp::Platform::FeatureFlags::MOUSE_GLOBAL_STATE;
			}
		}
#endif
		return feature_flags;
	}

	uint64_t PlatformInterface::get_performance_frequency() const
	{
		return SDL_GetPerformanceFrequency();
	}

	uint64_t PlatformInterface::get_performance_counter() const
	{
		return SDL_GetPerformanceCounter();
	}

	void PlatformInterface::show_cursor(bool show)
	{
		SDL_ShowCursor(show ? SDL_TRUE : SDL_FALSE);
	}

	void PlatformInterface::set_cursor(VadonApp::Platform::Cursor cursor)
	{
		SDL_SetCursor(m_cursors[Vadon::Utilities::to_integral(cursor)]);
	}

	void PlatformInterface::capture_mouse(bool capture)
	{
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
		// SDL_CaptureMouse() let the OS know e.g. that our imgui drag outside the SDL window boundaries shouldn't e.g. trigger other operations outside
		SDL_CaptureMouse(capture ? SDL_TRUE : SDL_FALSE);
#endif
	}

	void PlatformInterface::warp_mouse(const Vadon::Utilities::Vector2i& mouse_position)
	{
		SDL_WarpMouseInWindow(m_main_window.sdl_window, mouse_position.x, mouse_position.y);
	}

	Vadon::Utilities::Vector2i PlatformInterface::get_mouse_position() const
	{
		int mouse_x_global, mouse_y_global;
		SDL_GetGlobalMouseState(&mouse_x_global, &mouse_y_global);

		return Vadon::Utilities::Vector2i(mouse_x_global, mouse_y_global);
	}

	void PlatformInterface::set_clipboard_text(const char* text)
	{
		SDL_SetClipboardText(text);
	}

	const char* PlatformInterface::get_clipboard_text()
	{
		// Free the previous clipboard
		free_clipboard();

		m_clipboard = SDL_GetClipboardText();
		return m_clipboard;
	}

	bool PlatformInterface::initialize_internal()
	{
		// Initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			const std::string error_message = std::string("Failed to initialize SDL! SDL_Error: ") + SDL_GetError();
			log_error(error_message);
			return false;
		}

		const VadonApp::Platform::Configuration& platform_config = m_application.get_config().platform_config;
		const VadonApp::Platform::WindowInfo& main_window_info = platform_config.main_window_info;
		working_dir = SDL_GetBasePath();

		// Prepare arguments
		const int pos_x = (main_window_info.position.x >= 0) ? main_window_info.position.x : SDL_WINDOWPOS_UNDEFINED;
		const int pos_y = (main_window_info.position.y >= 0) ? main_window_info.position.y : SDL_WINDOWPOS_UNDEFINED;

		// Attempt to create the SDL window
		m_main_window.sdl_window = SDL_CreateWindow(main_window_info.title.c_str(), pos_x, pos_y, main_window_info.size.x, main_window_info.size.y, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
		if (!m_main_window.sdl_window)
		{
			// Something went wrong
			log_error("Failed to create SDL main window!");
			return false;
		}

		m_main_window.render_window.window = main_window_info;
		m_main_window.render_window.window.position = get_sdl_window_position(m_main_window.sdl_window); // Cache the actual starting position

		Vadon::Render::WindowInfo window_info;
		window_info.dimensions = get_sdl_window_size(m_main_window.sdl_window);
		window_info.platform_handle = get_window_handle();

		Vadon::Render::RenderTargetSystem& rt_system = m_application.get_engine_core().get_system<Vadon::Render::RenderTargetSystem>();
		m_main_window.render_window.render_handle = rt_system.add_window(window_info);

		if (!m_main_window.render_window.render_handle.is_valid())
		{
			// Something went wrong
			log_error("Failed to register main render window!\n");
			return false;
		}

		cache_window_drawable_size();

		// Register cursors
		m_cursors.fill(nullptr);
		for (int current_cursor_index = 0; current_cursor_index < Vadon::Utilities::to_integral(VadonApp::Platform::Cursor::CURSOR_COUNT); ++current_cursor_index)
		{
			const VadonApp::Platform::Cursor current_cursor = Vadon::Utilities::to_enum<VadonApp::Platform::Cursor>(current_cursor_index);
			m_cursors[current_cursor_index] = SDL_CreateSystemCursor(get_sdl_cursor(current_cursor));
		}

		log_message("SDL platform interface initialized successfully!\n");
		return true;
	}

	void PlatformInterface::shutdown_internal()
	{
		// Clean up cursors
		for (SDL_Cursor* current_cursor : m_cursors)
		{
			if (current_cursor)
			{
				SDL_FreeCursor(current_cursor);
			}
		}
		m_cursors.fill(nullptr);

		free_clipboard();

		if (m_main_window.sdl_window)
		{
			// Destroy window
			SDL_DestroyWindow(m_main_window.sdl_window);
			m_main_window.sdl_window = nullptr;
		}

		// Quit SDL subsystems
		SDL_Quit();
		log_message("SDL shut down successfully.\n");
	}

	VadonApp::Platform::WindowEvent PlatformInterface::handle_window_event(const SDL_Event& sdl_event)
	{
		VadonApp::Platform::WindowEvent window_event;
		window_event.type = convert_sdl_to_platform_window_event_type(static_cast<SDL_WindowEventID>(sdl_event.window.event));

		// TODO: handle all (relevant) events: https://wiki.libsdl.org/SDL2/SDL_WindowEvent
		switch (sdl_event.window.event)
		{
		case SDL_WINDOWEVENT_MOVED:
		{
			// Set the event data
			window_event.data1 = sdl_event.window.data1;
			window_event.data2 = sdl_event.window.data2;

			window_moved(Vadon::Utilities::Vector2i(window_event.data1, window_event.data2));
		}
		break;
		case SDL_WINDOWEVENT_RESIZED:
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		{
			// Set the event data
			window_event.data1 = sdl_event.window.data1;
			window_event.data2 = sdl_event.window.data2;

			// Cache the new size
			m_main_window.render_window.window.size = Vadon::Utilities::Vector2i(window_event.data1, window_event.data2);
			cache_window_drawable_size();
		}
		break;
		}

		return window_event;
	}

	void PlatformInterface::window_moved(const Vadon::Utilities::Vector2i& position)
	{
		// Cache the new position
		m_main_window.render_window.window.position = position;
	}

	void PlatformInterface::cache_window_drawable_size()
	{
		int drawable_width = 0;
		int drawable_height = 0;
		SDL_GL_GetDrawableSize(m_main_window.sdl_window, &drawable_width, &drawable_height);

		m_main_window.render_window.drawable_size = Vadon::Utilities::Vector2i(drawable_width, drawable_height);
	}

	void PlatformInterface::free_clipboard()
	{
		if (m_clipboard)
		{
			SDL_free(m_clipboard);
			m_clipboard = nullptr;
		}
	}
}