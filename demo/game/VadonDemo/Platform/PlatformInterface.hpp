#ifndef VADONDEMO_PLATFORM_PLATFORMINTERFACE_HPP
#define VADONDEMO_PLATFORM_PLATFORMINTERFACE_HPP
#include <VadonApp/Platform/Event/Event.hpp>
#include <VadonApp/Platform/Input/Input.hpp>
#include <VadonApp/Platform/Window/Window.hpp>
#include <array>
namespace VadonDemo::Core
{
	class GameCore;
}
namespace VadonDemo::Platform
{
	enum class GameInputAction
	{
		MOVE_UP,
		MOVE_DOWN,
		MOVE_LEFT,
		MOVE_RIGHT,
		UI_SELECT,
		ACTION_COUNT
	};

	class PlatformInterface
	{
	public:
		~PlatformInterface();

		VadonApp::Platform::InputActionHandle get_action(GameInputAction action) const { return m_input_actions[Vadon::Utilities::to_integral(action)]; }

		VadonApp::Platform::WindowHandle get_main_window() const { return m_main_window; }
		void toggle_fullscreen();
	private:
		PlatformInterface(Core::GameCore& game_core);

		bool initialize();
		void update();

		bool init_inputs();

		Core::GameCore& m_game_core;

		VadonApp::Platform::WindowHandle m_main_window;

		std::array<VadonApp::Platform::InputActionHandle, Vadon::Utilities::to_integral(GameInputAction::ACTION_COUNT)> m_input_actions;
		float m_dispatch_timer = 0.0f;

		friend Core::GameCore;
	};
}
#endif