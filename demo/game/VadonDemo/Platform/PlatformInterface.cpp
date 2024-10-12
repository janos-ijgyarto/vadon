#include <VadonDemo/Platform/PlatformInterface.hpp>

#include <VadonDemo/Core/GameCore.hpp>

#include <VadonApp/Core/Application.hpp>
#include <VadonApp/Platform/PlatformInterface.hpp>
#include <VadonApp/Platform/Input/InputSystem.hpp>

#include <array>

namespace
{
	enum class InputAction
	{
		MOVE_UP,
		MOVE_DOWN,
		MOVE_LEFT,
		MOVE_RIGHT,
		FIRE,
		ACTION_COUNT
	};

	constexpr float c_platform_dispatch_interval = 1.0f / 60.0f;
}

namespace VadonDemo::Platform
{
	struct PlatformInterface::Internal
	{
		Core::GameCore& m_game_core;

		std::array<VadonApp::Platform::InputActionHandle, Vadon::Utilities::to_integral(InputAction::ACTION_COUNT)> m_input_actions;
		float m_dispatch_timer = 0.0f;

		Internal(Core::GameCore& game_core)
			: m_game_core(game_core)
		{

		}

		bool initialize()
		{
			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
			VadonApp::Platform::InputSystem& input_system = engine_app.get_system<VadonApp::Platform::InputSystem>();

			{
				VadonApp::Platform::InputActionHandle move_up_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "move_up" });
				input_system.add_key_entry(move_up_action, VadonApp::Platform::KeyCode::UP);
				m_input_actions[Vadon::Utilities::to_integral(InputAction::MOVE_UP)] = move_up_action;
			}

			{
				VadonApp::Platform::InputActionHandle move_down_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "move_down" });
				input_system.add_key_entry(move_down_action, VadonApp::Platform::KeyCode::DOWN);
				m_input_actions[Vadon::Utilities::to_integral(InputAction::MOVE_DOWN)] = move_down_action;
			}

			{
				VadonApp::Platform::InputActionHandle move_left_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "move_left" });
				input_system.add_key_entry(move_left_action, VadonApp::Platform::KeyCode::LEFT);
				m_input_actions[Vadon::Utilities::to_integral(InputAction::MOVE_LEFT)] = move_left_action;
			}

			{
				VadonApp::Platform::InputActionHandle move_right_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "move_right" });
				input_system.add_key_entry(move_right_action, VadonApp::Platform::KeyCode::RIGHT);
				m_input_actions[Vadon::Utilities::to_integral(InputAction::MOVE_RIGHT)] = move_right_action;
			}

			{
				VadonApp::Platform::InputActionHandle fire_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "fire" });
				input_system.add_key_entry(fire_action, VadonApp::Platform::KeyCode::SPACE);
				m_input_actions[Vadon::Utilities::to_integral(InputAction::FIRE)] = fire_action;
			}

			return true;
		}

		void update()
		{
			m_dispatch_timer += m_game_core.get_delta_time();

			if (m_dispatch_timer > c_platform_dispatch_interval)
			{
				VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();

				VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();
				platform_interface.dispatch_events();

				m_dispatch_timer = 0.0f;
			}
		}
	};

	PlatformInterface::~PlatformInterface() = default;

	void PlatformInterface::move_window(Vadon::Utilities::Vector2i position)
	{
		VadonApp::Platform::PlatformInterface& platform_interface = m_internal->m_game_core.get_engine_app().get_system<VadonApp::Platform::PlatformInterface>();
		platform_interface.move_window(position);
	}

	void PlatformInterface::resize_window(Vadon::Utilities::Vector2i size)
	{
		VadonApp::Platform::PlatformInterface& platform_interface = m_internal->m_game_core.get_engine_app().get_system<VadonApp::Platform::PlatformInterface>();
		platform_interface.resize_window(size);
	}

	PlatformInterface::InputValues PlatformInterface::get_input_values() const
	{
		VadonApp::Core::Application& engine_app = m_internal->m_game_core.get_engine_app();
		VadonApp::Platform::InputSystem& input_system = engine_app.get_system<VadonApp::Platform::InputSystem>();

		return PlatformInterface::InputValues{
			.move_up = input_system.is_action_pressed(m_internal->m_input_actions[Vadon::Utilities::to_integral(InputAction::MOVE_UP)]),
			.move_down = input_system.is_action_pressed(m_internal->m_input_actions[Vadon::Utilities::to_integral(InputAction::MOVE_DOWN)]),
			.move_left = input_system.is_action_pressed(m_internal->m_input_actions[Vadon::Utilities::to_integral(InputAction::MOVE_LEFT)]),
			.move_right = input_system.is_action_pressed(m_internal->m_input_actions[Vadon::Utilities::to_integral(InputAction::MOVE_RIGHT)]),
			.fire = input_system.is_action_pressed(m_internal->m_input_actions[Vadon::Utilities::to_integral(InputAction::FIRE)])
		};
	}

	PlatformInterface::PlatformInterface(Core::GameCore& game_core)
		: m_internal(std::make_unique<Internal>(game_core))
	{

	}

	bool PlatformInterface::initialize()
	{
		return m_internal->initialize();
	}

	void PlatformInterface::update()
	{
		m_internal->update();
	}
}