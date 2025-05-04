#include <VadonDemo/Platform/PlatformInterface.hpp>

#include <VadonDemo/Core/GameCore.hpp>

#include <VadonApp/Core/Application.hpp>
#include <VadonApp/Platform/PlatformInterface.hpp>
#include <VadonApp/Platform/Input/InputSystem.hpp>

namespace
{
	constexpr float c_platform_dispatch_interval = 1.0f / 60.0f;

	// TODO: implement systems for setting this up based on command line, serialized config, etc.
	constexpr int c_screen_width = 1024;
	constexpr int c_screen_height = 768;
}

namespace VadonDemo::Platform
{
	PlatformInterface::~PlatformInterface() = default;

	void PlatformInterface::toggle_fullscreen()
	{
		VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
		VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();

		platform_interface.toggle_window_borderless_fullscreen(m_main_window);
	}

	PlatformInterface::PlatformInterface(Core::GameCore& game_core)
		: m_game_core(game_core)
	{

	}

	bool PlatformInterface::initialize()
	{
		VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();

		// FIXME: have client provide platform config
		VadonApp::Platform::WindowInfo main_window_info;
		main_window_info.title = "Vadon Demo"; // TODO: version numbering?
		main_window_info.position = Vadon::Utilities::Vector2i(-1, -1);
		main_window_info.size = Vadon::Utilities::Vector2i(c_screen_width, c_screen_height);

		main_window_info.flags = VadonApp::Platform::WindowFlags::SHOWN | VadonApp::Platform::WindowFlags::RESIZABLE;

		VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();
		m_main_window = platform_interface.create_window(main_window_info);

		if (m_main_window.is_valid() == false)
		{
			return false;
		}

		if (init_inputs() == false)
		{
			return false;
		}

		return true;
	}

	void PlatformInterface::update()
	{
		m_dispatch_timer += m_game_core.get_delta_time();

		if (m_dispatch_timer > c_platform_dispatch_interval)
		{
			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();

			VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();
			platform_interface.poll_events();

			m_dispatch_timer = 0.0f;
		}
	}

	bool PlatformInterface::init_inputs()
	{
		VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
		VadonApp::Platform::InputSystem& input_system = engine_app.get_system<VadonApp::Platform::InputSystem>();

		{
			VadonApp::Platform::InputActionHandle move_up_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "move_up" });
			input_system.add_key_entry(move_up_action, VadonApp::Platform::KeyCode::UP);
			m_input_actions[Vadon::Utilities::to_integral(GameInputAction::MOVE_UP)] = move_up_action;
		}

		{
			VadonApp::Platform::InputActionHandle move_down_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "move_down" });
			input_system.add_key_entry(move_down_action, VadonApp::Platform::KeyCode::DOWN);
			m_input_actions[Vadon::Utilities::to_integral(GameInputAction::MOVE_DOWN)] = move_down_action;
		}

		{
			VadonApp::Platform::InputActionHandle move_left_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "move_left" });
			input_system.add_key_entry(move_left_action, VadonApp::Platform::KeyCode::LEFT);
			m_input_actions[Vadon::Utilities::to_integral(GameInputAction::MOVE_LEFT)] = move_left_action;
		}

		{
			VadonApp::Platform::InputActionHandle move_right_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "move_right" });
			input_system.add_key_entry(move_right_action, VadonApp::Platform::KeyCode::RIGHT);
			m_input_actions[Vadon::Utilities::to_integral(GameInputAction::MOVE_RIGHT)] = move_right_action;
		}

		{
			VadonApp::Platform::InputActionHandle ui_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "ui_select" });
			input_system.add_mouse_entry(ui_action, VadonApp::Platform::MouseButton::LEFT);
			m_input_actions[Vadon::Utilities::to_integral(GameInputAction::UI_SELECT)] = ui_action;
		}

		return true;
	}
}