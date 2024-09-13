#ifndef VADONAPP_PRIVATE_PLATFORM_INPUT_INPUTSYSTEM_HPP
#define VADONAPP_PRIVATE_PLATFORM_INPUT_INPUTSYSTEM_HPP
#include <VadonApp/Platform/Input/InputSystem.hpp>
#include <VadonApp/Private/Platform/Input/Input.hpp>
#include <VadonApp/Platform/Event/Event.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>
namespace VadonApp::Private::Platform
{
	class InputSystem final : public VadonApp::Platform::InputSystem
	{
	public:
		InputActionHandle create_input_action(const InputActionInfo& action_info) override;
		InputActionHandle find_input_action(std::string_view name) const override;
		bool is_action_valid(InputActionHandle action_handle) const override { return m_action_pool.is_handle_valid(action_handle); }
		void remove_input_action(InputActionHandle action_handle) override;

		void add_mouse_entry(InputActionHandle action_handle, VadonApp::Platform::MouseButton mouse_button) override;
		void add_mouse_wheel_entry(InputActionHandle action_handle) override;
		void add_key_entry(InputActionHandle action_handle, VadonApp::Platform::KeyCode key) override;

		float get_action_strength(InputActionHandle action_handle) const override;
		bool is_action_pressed(InputActionHandle action_handle) const override;
	private:
		enum class InputType
		{
			KEY_PRESS,
			MOUSE_BUTTON,
			MOUSE_WHEEL,
			INVALID
			// TODO: others?
		};

		struct InputActionEntry
		{
			InputType type = InputType::INVALID;
			int32_t index = -1;
		};

		struct InputActionValue
		{
			InputActionEntry entry;
			float value = 0.0f;
		};

		struct InputActionData
		{
			InputActionInfo info;
			std::vector<InputActionEntry> entries;
		};

		InputSystem(VadonApp::Core::Application& application);

		bool initialize();
		void shutdown();
		void reset_states();

		void process_platform_events(const VadonApp::Platform::PlatformEventList& platform_events);
		static InputActionValue parse_input_event(const VadonApp::Platform::PlatformEvent& input_event);

		Vadon::Utilities::ObjectPool<VadonApp::Platform::InputAction, InputActionData> m_action_pool;
		std::unordered_map<std::string, InputActionHandle> m_action_lookup;

		// FIXME: compress this data somehow?
		// TODO: support other inputs!
		std::array<bool, Vadon::Utilities::to_integral(VadonApp::Platform::KeyCode::KEY_COUNT)> m_key_states = {};
		std::array<bool, Vadon::Utilities::to_integral(VadonApp::Platform::MouseButton::BUTTON_COUNT)> m_mouse_button_states = {};
		float m_mouse_wheel_state = 0.0f;

		friend class PlatformInterface;
	};
}
#endif