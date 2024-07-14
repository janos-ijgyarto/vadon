#ifndef VADONDEMO_CORE_GAMECORE_HPP
#define VADONDEMO_CORE_GAMECORE_HPP
#include <memory>

namespace VadonApp::Core
{
	class Application;
}

namespace VadonDemo::Model
{
	class Model;
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

namespace Vadon::ECS
{
	class World;
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

		Model::Model& get_model();

		Vadon::ECS::World& get_ecs_world();
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif 