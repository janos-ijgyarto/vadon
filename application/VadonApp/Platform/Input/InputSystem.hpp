#ifndef VADONAPP_PLATFORM_INPUT_INPUTSYSTEM_HPP
#define VADONAPP_PLATFORM_INPUT_INPUTSYSTEM_HPP
#include <VadonApp/Platform/Module.hpp>
#include <VadonApp/Platform/Input/Mouse.hpp>
#include <VadonApp/Platform/Input/Keyboard.hpp>
#include <VadonApp/Platform/Input/Input.hpp>
namespace VadonApp::Platform
{
	class InputSystem : public PlatformSystem<InputSystem>
	{
	public:
		virtual InputActionHandle create_input_action(const InputActionInfo& action_info) = 0;
		virtual InputActionHandle find_input_action(std::string_view name) const = 0;
		virtual bool is_action_valid(InputActionHandle action_handle) const = 0;
		virtual void remove_input_action(InputActionHandle action_handle) = 0;

		virtual void add_mouse_entry(InputActionHandle action_handle, MouseButton mouse_button) = 0;
		virtual void add_mouse_wheel_entry(InputActionHandle action_handle) = 0;
		virtual void add_key_entry(InputActionHandle action_handle, KeyCode key) = 0;

		virtual float get_action_strength(InputActionHandle action_handle) const = 0;
		virtual bool is_action_pressed(InputActionHandle action_handle) const = 0;

		virtual void update() = 0;
	protected:
		InputSystem(Core::Application& application)
			: System(application)
		{}
	};
}
#endif