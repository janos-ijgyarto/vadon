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
		CAMERA_UP,
		CAMERA_DOWN,
		CAMERA_LEFT,
		CAMERA_RIGHT,
		CAMERA_ZOOM,
		ACTION_COUNT
	};
}

namespace VadonDemo::Platform
{
	struct PlatformInterface::Internal
	{
		Core::GameCore& m_game_core;

		std::vector<PlatformEventCallback> m_event_callbacks;
		std::array<VadonApp::Platform::InputActionHandle, Vadon::Utilities::to_integral(InputAction::ACTION_COUNT)> m_input_actions;

		Internal(Core::GameCore& game_core)
			: m_game_core(game_core)
		{

		}

		bool initialize()
		{
			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
			VadonApp::Platform::InputSystem& input_system = engine_app.get_system<VadonApp::Platform::InputSystem>();

			{
				VadonApp::Platform::InputActionHandle camera_up_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "camera_up" });
				input_system.add_key_entry(camera_up_action, VadonApp::Platform::KeyCode::UP);
				m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_UP)] = camera_up_action;
			}

			{
				VadonApp::Platform::InputActionHandle camera_down_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "camera_down" });
				input_system.add_key_entry(camera_down_action, VadonApp::Platform::KeyCode::DOWN);
				m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_DOWN)] = camera_down_action;
			}

			{
				VadonApp::Platform::InputActionHandle camera_left_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "camera_left" });
				input_system.add_key_entry(camera_left_action, VadonApp::Platform::KeyCode::LEFT);
				m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_LEFT)] = camera_left_action;
			}

			{
				VadonApp::Platform::InputActionHandle camera_right_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "camera_right" });
				input_system.add_key_entry(camera_right_action, VadonApp::Platform::KeyCode::RIGHT);
				m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_RIGHT)] = camera_right_action;
			}

			{
				VadonApp::Platform::InputActionHandle camera_zoom_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "camera_zoom" });
				input_system.add_mouse_wheel_entry(camera_zoom_action);
				m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_ZOOM)] = camera_zoom_action;
			}

			return true;
		}

		void update()
		{
			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();

			VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();
			const VadonApp::Platform::PlatformEventList platform_events = platform_interface.read_events();

			VadonApp::Platform::InputSystem& input_system = engine_app.get_system<VadonApp::Platform::InputSystem>();
			input_system.dispatch_platform_events(platform_events);

			if (platform_events.empty())
			{
				return;
			}

			// Propagate to all callbacks (it's their responsibility to manage data races, if needed)
			for (const PlatformEventCallback& current_callback : m_event_callbacks)
			{
				current_callback(platform_events);
			}
		}
	};

	PlatformInterface::~PlatformInterface() = default;

	void PlatformInterface::register_event_callback(const PlatformEventCallback& callback)
	{
		m_internal->m_event_callbacks.push_back(callback);
	}

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
			.camera_up = input_system.is_action_pressed(m_internal->m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_UP)]),
			.camera_down = input_system.is_action_pressed(m_internal->m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_DOWN)]),
			.camera_left = input_system.is_action_pressed(m_internal->m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_LEFT)]),
			.camera_right = input_system.is_action_pressed(m_internal->m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_RIGHT)]),
			.camera_zoom = input_system.get_action_strength(m_internal->m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_ZOOM)]),
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