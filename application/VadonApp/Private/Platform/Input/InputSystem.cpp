#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Private/Platform/Input/InputSystem.hpp>

#include <VadonApp/Platform/PlatformInterface.hpp>

#include <Vadon/Utilities/Data/Visitor.hpp>

#include <format>

namespace VadonApp::Private::Platform
{
	InputActionHandle InputSystem::create_input_action(const InputActionInfo& action_info)
	{
		InputActionHandle new_action_handle;
		if (action_info.name.empty() == true)
		{
			log_error("Input system: cannot register action with no name!\n");
			return new_action_handle;
		}

		if(find_input_action(action_info.name).is_valid() == true)
		{
			log_error(std::format("Input system: input action with name \"{}\" already registered!\n", action_info.name));
			return new_action_handle;
		}

		new_action_handle = m_action_pool.add();

		InputActionData& action_data = m_action_pool.get(new_action_handle);
		action_data.info = action_info;

		return new_action_handle;
	}

	InputActionHandle InputSystem::find_input_action(std::string_view name) const
	{
		auto action_it = m_action_lookup.find(std::string(name));
		if (action_it != m_action_lookup.end())
		{
			return action_it->second;
		}

		return InputActionHandle();
	}

	void InputSystem::remove_input_action(InputActionHandle action_handle)
	{
		// TODO: cleanup?
		InputActionData& action_data = m_action_pool.get(action_handle);		
		m_action_lookup.erase(action_data.info.name);

		m_action_pool.remove(action_handle);
	}

	void InputSystem::add_mouse_entry(InputActionHandle action_handle, VadonApp::Platform::MouseButton mouse_button)
	{
		InputActionData& action_data = m_action_pool.get(action_handle);
		action_data.entries.push_back(InputActionEntry{ .type = InputType::MOUSE_BUTTON, .index = Vadon::Utilities::to_integral(mouse_button) });
	}

	void InputSystem::add_mouse_wheel_entry(InputActionHandle action_handle)
	{
		InputActionData& action_data = m_action_pool.get(action_handle);
		action_data.entries.push_back(InputActionEntry{ .type = InputType::MOUSE_WHEEL, .index = 0 });
	}

	void InputSystem::add_key_entry(InputActionHandle action_handle, VadonApp::Platform::KeyCode key)
	{
		InputActionData& action_data = m_action_pool.get(action_handle);
		action_data.entries.push_back(InputActionEntry{ .type = InputType::KEY_PRESS, .index = Vadon::Utilities::to_integral(key) });
	}

	float InputSystem::get_action_strength(InputActionHandle action_handle) const
	{
		const InputActionData& action_data = m_action_pool.get(action_handle);
		for (const InputActionEntry& current_entry : action_data.entries)
		{
			switch (current_entry.type)
			{
			case InputType::MOUSE_BUTTON:
			{
				if (m_mouse_button_states[current_entry.index] == true)
				{
					return 1.0f;
				}
			}
				break;
			case InputType::MOUSE_WHEEL:
				return m_mouse_wheel_state;
			case InputType::KEY_PRESS:
			{
				if (m_key_states[current_entry.index] == true)
				{
					return 1.0f;
				}
			}
			break;
			}
		}

		return 0.0f;
	}

	bool InputSystem::is_action_pressed(InputActionHandle action_handle) const
	{
		const InputActionData& action_data = m_action_pool.get(action_handle);
		for (const InputActionEntry& current_entry : action_data.entries)
		{
			switch (current_entry.type)
			{
			case InputType::MOUSE_BUTTON:
			{
				if (m_mouse_button_states[current_entry.index] == true)
				{
					return true;
				}
			}
				break;
			case InputType::KEY_PRESS:
			{
				if (m_key_states[current_entry.index] == true)
				{
					return true;
				}
			}
				break;
			}
		}

		return false;
	}

	void InputSystem::update()
	{
		reset_states();

		VadonApp::Platform::PlatformInterface& platform_interface = m_application.get_system<VadonApp::Platform::PlatformInterface>();
		for (const VadonApp::Platform::PlatformEvent& current_event : platform_interface.poll_events())
		{
			const InputActionValue action_value = parse_input_event(current_event);
			switch (action_value.entry.type)
			{
			case InputType::MOUSE_BUTTON:
				m_mouse_button_states[action_value.entry.index] = action_value.value > 0.0f;
				break;
			case InputType::MOUSE_WHEEL:
				m_mouse_wheel_state = action_value.value;
				break;
			case InputType::KEY_PRESS:
				m_key_states[action_value.entry.index] = action_value.value > 0.0f;
				break;
			}
		}
	}

	InputSystem::InputSystem(VadonApp::Core::Application& application)
		: VadonApp::Platform::InputSystem(application)
	{}

	bool InputSystem::initialize()
	{
		// TODO: anything?
		return true;
	}

	void InputSystem::shutdown()
	{
		// TODO: anything?
	}

	void InputSystem::reset_states()
	{
		// FIXME: should we reset any other states?
		m_mouse_wheel_state = 0.0f;
	}

	InputSystem::InputActionValue InputSystem::parse_input_event(const VadonApp::Platform::PlatformEvent& input_event)
	{
		static constexpr auto input_event_visitor = Vadon::Utilities::VisitorOverloadList{
			[](const VadonApp::Platform::MouseButtonEvent& mouse_button)
			{
				return InputActionValue{.entry = InputActionEntry{.type = InputType::MOUSE_BUTTON, .index = Vadon::Utilities::to_integral(mouse_button.button) }, .value = mouse_button.down ? 1.0f : -1.0f};
			},
			[](const VadonApp::Platform::MouseWheelEvent& mouse_wheel_event)
			{
				return InputActionValue{.entry = InputActionEntry{.type = InputType::MOUSE_WHEEL, .index = 0 }, .value = mouse_wheel_event.y > 0 ? 1.0f : -1.0f };
			},
			[](const VadonApp::Platform::KeyboardEvent& keyboard_event)
			{
				return InputActionValue{.entry = InputActionEntry{.type = InputType::KEY_PRESS, .index = Vadon::Utilities::to_integral(keyboard_event.key)}, .value = keyboard_event.down ? 1.0f : -1.0f };
			},
			// TODO: support other inputs?
			[](auto) { return InputActionValue{}; }
		};

		return std::visit(input_event_visitor, input_event);
	}
}