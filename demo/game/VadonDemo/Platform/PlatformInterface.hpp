#ifndef VADONDEMO_PLATFORM_PLATFORMINTERFACE_HPP
#define VADONDEMO_PLATFORM_PLATFORMINTERFACE_HPP
#include <Vadon/Utilities/Math/Vector.hpp>
#include <VadonApp/Platform/Event/Event.hpp>
#include <VadonApp/Platform/Input/Input.hpp>
#include <VadonApp/Platform/Window/Window.hpp>
#include <memory>
namespace VadonDemo::Core
{
	class GameCore;
}
namespace VadonDemo::Platform
{
	class PlatformInterface
	{
	public:
		struct InputValues
		{
			bool move_up = false;
			bool move_down = false;
			bool move_left = false;
			bool move_right = false;
			bool fire = false;
		};

		~PlatformInterface();

		InputValues get_input_values() const;

		VadonApp::Platform::WindowHandle get_main_window() const;
		void toggle_fullscreen();
	private:
		PlatformInterface(Core::GameCore& game_core);

		bool initialize();
		void update();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::GameCore;
	};
}
#endif