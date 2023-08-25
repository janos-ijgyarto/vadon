#ifndef VADONDEMO_CORE_GAMECORE_HPP
#define VADONDEMO_CORE_GAMECORE_HPP
#include <memory>

namespace VadonApp::Core
{
	class Application;
}

namespace VadonDemo::Platform
{
	class PlatformInterface;
}

namespace VadonDemo::Render
{
	class RenderSystem;
}

namespace VadonDemo::UI
{
	class MainWindow;
}

namespace VadonDemo::Core
{
	// FIXME: make separate interface so we don't expose "execute" to subsystems
	class GameCore
	{
	public:
		GameCore();
		~GameCore();

		int execute(int argc, char* argv[]);

		VadonApp::Core::Application& get_engine_app();

		Platform::PlatformInterface& get_platform_interface();
		Render::RenderSystem& get_render_system();
		UI::MainWindow& get_main_window();
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif 