#ifndef VADONDEMO_CORE_GAMECORE_HPP
#define VADONDEMO_CORE_GAMECORE_HPP
#include <Vadon/Core/File/RootDirectory.hpp>
#include <memory>

namespace VadonApp::Core
{
	class Application;
}

namespace VadonDemo::Model
{
	class GameModel;
}

namespace VadonDemo::Platform
{
	class PlatformInterface;
}

namespace VadonDemo::Render
{
	class RenderSystem;
}

namespace VadonDemo::View
{
	class GameView;
}

namespace VadonDemo::UI
{
	class UISystem;
}

namespace Vadon::Core
{
	class EngineEnvironment;
	struct Project;
}

namespace Vadon::ECS
{
	class World;
}

namespace VadonDemo::Core
{
	class Core;

	// FIXME: make separate interface so we don't expose "execute" to subsystems
	class GameCore
	{
	public:
		GameCore(Vadon::Core::EngineEnvironment& environment);
		~GameCore();

		int execute(int argc, char* argv[]);

		VadonApp::Core::Application& get_engine_app();

		Platform::PlatformInterface& get_platform_interface();
		Render::RenderSystem& get_render_system();
		UI::UISystem& get_ui_system();

		float get_delta_time() const;

		Core& get_core();
		Model::GameModel& get_model();
		View::GameView& get_view();

		Vadon::ECS::World& get_ecs_world();

		const Vadon::Core::Project& get_project_info() const;
		Vadon::Core::RootDirectoryHandle get_project_root_dir() const;

		// FIXME: implement a proper CLI parser!
		bool has_command_line_arg(std::string_view name) const;
		std::string get_command_line_arg(std::string_view name) const;

		void request_shutdown();
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif 