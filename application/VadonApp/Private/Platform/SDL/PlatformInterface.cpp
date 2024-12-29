#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Private/Platform/SDL/PlatformInterface.hpp>

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
			using WindowEventType = VadonApp::Platform::WindowEventType;

			// FIXME: replace with an array?
			switch (event_id)
			{
			case SDL_WindowEventID::SDL_WINDOWEVENT_SHOWN:
				return WindowEventType::SHOWN;
			case SDL_WindowEventID::SDL_WINDOWEVENT_HIDDEN:
				return WindowEventType::HIDDEN;
			case SDL_WindowEventID::SDL_WINDOWEVENT_EXPOSED:
				return WindowEventType::EXPOSED;
			case SDL_WindowEventID::SDL_WINDOWEVENT_MOVED:
				return WindowEventType::MOVED;
			case SDL_WindowEventID::SDL_WINDOWEVENT_RESIZED:
				return WindowEventType::RESIZED;
			case SDL_WindowEventID::SDL_WINDOWEVENT_SIZE_CHANGED:
				return WindowEventType::SIZE_CHANGED;
			case SDL_WindowEventID::SDL_WINDOWEVENT_MINIMIZED:
				return WindowEventType::ENTER;
			case SDL_WindowEventID::SDL_WINDOWEVENT_MAXIMIZED:
				return WindowEventType::ENTER;
			case SDL_WindowEventID::SDL_WINDOWEVENT_RESTORED:
				return WindowEventType::ENTER;
			case SDL_WindowEventID::SDL_WINDOWEVENT_ENTER:
				return WindowEventType::ENTER;
			case SDL_WindowEventID::SDL_WINDOWEVENT_LEAVE:
				return WindowEventType::LEAVE;
			case SDL_WindowEventID::SDL_WINDOWEVENT_FOCUS_GAINED:
				return WindowEventType::FOCUS_GAINED;
			case SDL_WindowEventID::SDL_WINDOWEVENT_FOCUS_LOST:
				return WindowEventType::FOCUS_LOST;
			case SDL_WindowEventID::SDL_WINDOWEVENT_CLOSE:
				return WindowEventType::CLOSE;
			case SDL_WindowEventID::SDL_WINDOWEVENT_DISPLAY_CHANGED:
				return WindowEventType::DISPLAY_CHANGED;
			}

			return WindowEventType::NONE;
		}

		constexpr WindowFlags convert_sdl_to_platform_window_flags(SDL_WindowFlags sdl_flags)
		{
			WindowFlags window_flags = WindowFlags::NONE;

			VADON_START_BITMASK_SWITCH(sdl_flags)
			{
			case SDL_WINDOW_FULLSCREEN:
				window_flags |= WindowFlags::FULLSCREEN;
				break;
			case SDL_WINDOW_SHOWN:
				window_flags |= WindowFlags::SHOWN;
				break;
			case SDL_WINDOW_HIDDEN:
				window_flags |= WindowFlags::HIDDEN;
				break;
			case SDL_WINDOW_BORDERLESS:
				window_flags |= WindowFlags::BORDERLESS;
				break;
			case SDL_WINDOW_RESIZABLE:
				window_flags |= WindowFlags::RESIZABLE;
				break;
			case SDL_WINDOW_MINIMIZED:
				window_flags |= WindowFlags::MINIMIZED;
				break;
			case SDL_WINDOW_MAXIMIZED:
				window_flags |= WindowFlags::MAXIMIZED;
				break;
			case SDL_WINDOW_MOUSE_GRABBED:
				window_flags |= WindowFlags::MOUSE_GRABBED;
				break;
			case SDL_WINDOW_INPUT_FOCUS:
				window_flags |= WindowFlags::INPUT_FOCUS;
				break;
			case SDL_WINDOW_MOUSE_FOCUS:
				window_flags |= WindowFlags::MOUSE_FOCUS;
				break;
			case SDL_WINDOW_MOUSE_CAPTURE:
				window_flags |= WindowFlags::MOUSE_CAPTURE;
				break;
			case SDL_WINDOW_ALWAYS_ON_TOP:
				window_flags |= WindowFlags::ALWAYS_ON_TOP;
				break;
			case SDL_WINDOW_SKIP_TASKBAR:
				window_flags |= WindowFlags::SKIP_TASKBAR;
				break;
			case SDL_WINDOW_KEYBOARD_GRABBED:
				window_flags |= WindowFlags::KEYBOARD_GRABBED;
				break;
			}

			return window_flags;
		}

		constexpr SDL_WindowFlags convert_platform_to_sdl_window_flags(WindowFlags window_flags)
		{
			Uint32 sdl_flags = 0;

			VADON_START_BITMASK_SWITCH(window_flags)
			{
			case WindowFlags::FULLSCREEN:
				sdl_flags |= SDL_WINDOW_FULLSCREEN;
				break;
			case WindowFlags::SHOWN:
				sdl_flags |= SDL_WINDOW_SHOWN;
				break;
			case WindowFlags::HIDDEN:
				sdl_flags |= SDL_WINDOW_HIDDEN;
				break;
			case WindowFlags::BORDERLESS:
				sdl_flags |= SDL_WINDOW_BORDERLESS;
				break;
			case WindowFlags::RESIZABLE:
				sdl_flags |= SDL_WINDOW_RESIZABLE;
				break;
			case WindowFlags::MINIMIZED:
				sdl_flags |= SDL_WINDOW_MINIMIZED;
				break;
			case WindowFlags::MAXIMIZED:
				sdl_flags |= SDL_WINDOW_MAXIMIZED;
				break;
			case WindowFlags::MOUSE_GRABBED:
				sdl_flags |= SDL_WINDOW_MOUSE_GRABBED;
				break;
			case WindowFlags::INPUT_FOCUS:
				sdl_flags |= SDL_WINDOW_INPUT_FOCUS;
				break;
			case WindowFlags::MOUSE_FOCUS:
				sdl_flags |= SDL_WINDOW_MOUSE_FOCUS;
				break;
			case WindowFlags::MOUSE_CAPTURE:
				sdl_flags |= SDL_WINDOW_MOUSE_CAPTURE;
				break;
			case WindowFlags::ALWAYS_ON_TOP:
				sdl_flags |= SDL_WINDOW_ALWAYS_ON_TOP;
				break;
			case WindowFlags::SKIP_TASKBAR:
				sdl_flags |= SDL_WINDOW_SKIP_TASKBAR;
				break;
			case WindowFlags::KEYBOARD_GRABBED:
				sdl_flags |= SDL_WINDOW_KEYBOARD_GRABBED;
				break;
			}

			return static_cast<SDL_WindowFlags>(sdl_flags);
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

	WindowHandle PlatformInterface::create_window(const WindowInfo& window_info)
	{
		// Prepare arguments
		const int pos_x = (window_info.position.x >= 0) ? window_info.position.x : SDL_WINDOWPOS_UNDEFINED;
		const int pos_y = (window_info.position.y >= 0) ? window_info.position.y : SDL_WINDOWPOS_UNDEFINED;
		const SDL_WindowFlags window_flags = convert_platform_to_sdl_window_flags(window_info.flags);

		// Attempt to create the SDL window
		SDL_Window* sdl_window = SDL_CreateWindow(window_info.title.c_str(), pos_x, pos_y, window_info.size.x, window_info.size.y, window_flags);
		if (sdl_window == nullptr)
		{
			// Something went wrong
			log_error("Platform interface: failed to create SDL window!\n");
			return WindowHandle();
		}

		WindowHandle new_window_handle = m_window_pool.add();
		SDLWindow& new_window = m_window_pool.get(new_window_handle);

		new_window.sdl_window = sdl_window;

		// Add to lookup
		const WindowID new_window_id = SDL_GetWindowID(sdl_window);
		m_window_lookup[new_window_id] = new_window_handle;
		
		return new_window_handle;
	}

	WindowHandle PlatformInterface::find_window(WindowID window_id) const
	{
		auto window_it = m_window_lookup.find(window_id);
		if (window_it != m_window_lookup.end())
		{
			return window_it->second;
		}
		return WindowHandle();
	}

	WindowID PlatformInterface::get_window_id(WindowHandle window_handle) const
	{
		const SDLWindow& window = m_window_pool.get(window_handle);
		return SDL_GetWindowID(window.sdl_window);
	}

	std::string PlatformInterface::get_window_title(WindowHandle window_handle) const
	{
		const SDLWindow& window = m_window_pool.get(window_handle);
		return SDL_GetWindowTitle(window.sdl_window);
	}

	void PlatformInterface::set_window_title(WindowHandle window_handle, std::string_view title)
	{
		SDLWindow& window = m_window_pool.get(window_handle);
		SDL_SetWindowTitle(window.sdl_window, title.data());
	}

	Vadon::Utilities::Vector2i PlatformInterface::get_window_position(WindowHandle window_handle) const
	{
		const SDLWindow& window = m_window_pool.get(window_handle);
		return get_sdl_window_position(window.sdl_window);
	}

	void PlatformInterface::set_window_position(WindowHandle window_handle, const Vadon::Utilities::Vector2i position)
	{
		SDLWindow& window = m_window_pool.get(window_handle);
		SDL_SetWindowPosition(window.sdl_window, position.x, position.y);
	}

	Vadon::Utilities::Vector2i PlatformInterface::get_window_size(WindowHandle window_handle) const
	{
		const SDLWindow& window = m_window_pool.get(window_handle);
		return get_sdl_window_size(window.sdl_window);
	}

	void PlatformInterface::set_window_size(WindowHandle window_handle, const Vadon::Utilities::Vector2i size)
	{
		SDLWindow& window = m_window_pool.get(window_handle);
		SDL_SetWindowSize(window.sdl_window, size.x, size.y);
	}

	WindowFlags PlatformInterface::get_window_flags(WindowHandle window_handle) const
	{
		const SDLWindow& window = m_window_pool.get(window_handle);
		return convert_sdl_to_platform_window_flags(static_cast<SDL_WindowFlags>(SDL_GetWindowFlags(window.sdl_window)));
	}

	PlatformWindowHandle PlatformInterface::get_platform_window_handle(WindowHandle window_handle) const
	{
		const SDLWindow& window = m_window_pool.get(window_handle);

		// Get the HWND from the SDL window
		// FIXME: branch on OS!!!
		SDL_SysWMinfo sdl_wm_info;
		SDL_GetVersion(&sdl_wm_info.version);
		SDL_GetWindowWMInfo(window.sdl_window, &sdl_wm_info);

		return sdl_wm_info.info.win.window;
	}

	Vadon::Utilities::Vector2i PlatformInterface::get_window_drawable_size(WindowHandle window_handle) const
	{
		const SDLWindow& window = m_window_pool.get(window_handle);

		int drawable_width = 0;
		int drawable_height = 0;
		SDL_GL_GetDrawableSize(window.sdl_window, &drawable_width, &drawable_height);

		return Vadon::Utilities::Vector2i(drawable_width, drawable_height);
	}

	bool PlatformInterface::is_window_focused(WindowHandle window_handle) const
	{
		const SDLWindow& window = m_window_pool.get(window_handle);
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
		SDL_Window* focused_window = SDL_GetKeyboardFocus();
		return (window.sdl_window == focused_window);
#else
		return ((SDL_GetWindowFlags(window.sdl_window) & SDL_WINDOW_INPUT_FOCUS) != 0); // SDL 2.0.3 and non-windowed systems: single-viewport only
#endif
	}

	void PlatformInterface::poll_events()
	{
		m_platform_events.clear();

		SDL_Event sdl_event;
		while (SDL_PollEvent(&sdl_event) != 0)
		{
			switch (sdl_event.type)
			{
			case SDL_QUIT:
				m_platform_events.emplace_back(VadonApp::Platform::QuitEvent{});
				break;
			case SDL_WINDOWEVENT:
			{
				VadonApp::Platform::WindowEvent window_event = handle_window_event(sdl_event);
				m_platform_events.emplace_back(window_event);
			}
			break;
			case SDL_TEXTINPUT:
			{
				VadonApp::Platform::TextInputEvent text_input;
				text_input.text = sdl_event.text.text;

				m_platform_events.emplace_back(text_input);
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

				m_platform_events.emplace_back(keyboard_event);
			}
			break;
			case SDL_MOUSEMOTION:
			{
				VadonApp::Platform::MouseMotionEvent motion_event;
				motion_event.position.x = sdl_event.motion.x;
				motion_event.position.y = sdl_event.motion.y;
				motion_event.relative_motion.x = sdl_event.motion.xrel;
				motion_event.relative_motion.y = sdl_event.motion.yrel;

				m_platform_events.emplace_back(motion_event);
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

				m_platform_events.emplace_back(button_event);
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

				m_platform_events.emplace_back(wheel_event);
			}
			break;
			}
		}

		// Dispatch to callbacks
		for (const EventCallback& current_callback : m_event_callbacks)
		{
			current_callback(m_platform_events);
		}
	}

	void PlatformInterface::register_event_callback(EventCallback callback)
	{
		m_event_callbacks.push_back(callback);
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

	void PlatformInterface::warp_mouse(WindowHandle window_handle, const Vadon::Utilities::Vector2i& mouse_position)
	{
		const SDLWindow& window = m_window_pool.get(window_handle);
		SDL_WarpMouseInWindow(window.sdl_window, mouse_position.x, mouse_position.y);
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

	bool PlatformInterface::internal_initialize()
	{
		// Initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			const std::string error_message = std::string("Failed to initialize SDL! SDL_Error: ") + SDL_GetError();
			log_error(error_message);
			return false;
		}

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

	void PlatformInterface::internal_shutdown()
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

		// Destroy windows
		for (auto window_pair : m_window_pool)
		{
			SDL_DestroyWindow(window_pair.second->sdl_window);
			window_pair.second->sdl_window = nullptr;
		}

		// Quit SDL subsystems
		SDL_Quit();
		log_message("SDL shut down successfully.\n");
	}

	VadonApp::Platform::WindowEvent PlatformInterface::handle_window_event(const SDL_Event& sdl_event)
	{
		VadonApp::Platform::WindowEvent window_event;
		window_event.type = convert_sdl_to_platform_window_event_type(static_cast<SDL_WindowEventID>(sdl_event.window.event));
		window_event.window_id = sdl_event.window.windowID;

		// TODO: handle all (relevant) events: https://wiki.libsdl.org/SDL2/SDL_WindowEvent
		switch (sdl_event.window.event)
		{
		case SDL_WINDOWEVENT_MOVED:
		{
			// Set the event data
			window_event.data1 = sdl_event.window.data1;
			window_event.data2 = sdl_event.window.data2;
		}
		break;
		case SDL_WINDOWEVENT_RESIZED:
		{
			// Set the event data
			window_event.data1 = sdl_event.window.data1;
			window_event.data2 = sdl_event.window.data2;
		}
		break;
		case SDL_WINDOWEVENT_DISPLAY_CHANGED:
		{
			window_event.data1 = sdl_event.window.data1;
		}
		break;
		}

		return window_event;
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