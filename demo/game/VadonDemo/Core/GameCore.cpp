#include <VadonDemo/Core/GameCore.hpp>

#include <VadonDemo/Model/GameModel.hpp>
#include <VadonDemo/Platform/PlatformInterface.hpp>
#include <VadonDemo/Render/RenderSystem.hpp>
#include <VadonDemo/UI/UISystem.hpp>
#include <VadonDemo/View/GameView.hpp>

#include <VadonApp/Core/Application.hpp>

#include <VadonApp/Platform/PlatformInterface.hpp>

#include <VadonApp/UI/UISystem.hpp>
#include <VadonApp/UI/Console.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/Core/Environment.hpp>
#include <Vadon/Core/Project/Project.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <chrono>
#include <format>

namespace VadonDemo::Core
{
	namespace
	{
		using Clock = std::chrono::steady_clock;
		using TimePoint = std::chrono::time_point<Clock>;
		using Duration = std::chrono::duration<float>;
	}

	struct GameCore::Internal
	{
		Model::GameModel m_game_model;
		Platform::PlatformInterface m_platform_interface;
		Render::RenderSystem m_render_system;
		UI::UISystem m_ui_system;
		View::GameView m_game_view;

		VadonApp::Core::Application::Instance m_engine_app;

		Vadon::ECS::World m_ecs_world;

		// FIXME: implement a proper CLI parser!
		std::string m_program_name;
		std::unordered_map<std::string, std::string> m_command_line_args;

		TimePoint m_last_time_point;
		float m_delta_time = 0.0f;

		bool m_shutdown_requested = false;

		Internal(GameCore& game_core, Vadon::Core::EngineEnvironment& environment)
			: m_game_model(game_core, environment)
			, m_platform_interface(game_core)
			, m_render_system(game_core)
			, m_ui_system(game_core)
			, m_game_view(game_core)
		{

		}

		bool initialize(int argc, char* argv[])
		{
			// FIXME: implement a proper CLI parser!
			parse_command_line(argc, argv);

			if (init_engine_application(argc, argv) == false)
			{
				Vadon::Core::Logger::log_error("Failed to initialize engine application!\n");
				return false;
			}

			if (!m_platform_interface.initialize())
			{
				return false;
			}

			if (!m_render_system.initialize())
			{
				return false;
			}

			if (m_ui_system.initialize() == false)
			{
				return false;
			}

			// All subsystems initialized, now we initialize the game model and view
			if (m_game_model.initialize() == false)
			{
				return false;
			}

			if (m_game_view.initialize() == false)
			{
				return false;
			}

			// FIXME: have to do this separately because all the types are only registered at this stage!
			if (m_game_model.init_database() == false)
			{
				return false;
			}

			// FIXME: currently have to give UUID base64 string. Allow referencing scenes via path!
			m_ui_system.register_console_command("load", 
				[this](std::string_view scene_id_string)
				{
					Vadon::Scene::SceneID scene_id;
					if (scene_id.from_base64_string(scene_id_string) == true)
					{
						m_game_model.load_level(Model::GameModel::LevelConfiguration{ .scene_id = scene_id });
					}
				}
			);

			m_ui_system.register_console_command("pause",
				[this](std::string_view value)
				{
					// TODO: error if param is incorrect?
					if (value.empty() == true)
					{
						return;
					}

					if (value == "1")
					{
						m_game_model.set_paused(true);
					}
					else if (value == "0")
					{
						m_game_model.set_paused(false);
					}
				}
			);

			m_ui_system.register_console_command("quit",
				[this](std::string_view)
				{
					m_game_model.quit_level();
				}
			);

			Vadon::Core::Logger::log_message("Vadon Demo app initialized.\n");

			m_last_time_point = Clock::now();

			return true;
		}

		void parse_command_line(int argc, char* argv[])
		{
			m_program_name = argv[0];

			for (int32_t current_arg_index = 1; current_arg_index < argc; ++current_arg_index)
			{
				parse_command_line_argument(argv[current_arg_index]);
			}
		}

		void parse_command_line_argument(const char* argument_ptr)
		{
			const std::string argument_string(argument_ptr);

			const size_t equals_char_offset = argument_string.find('=');

			// TODO: check that the arg isn't just whitespace

			if (equals_char_offset == std::string::npos)
			{
				// No equals char found, assume it's a command
				m_command_line_args.emplace(argument_string, "");
				return;
			}

			// Emplace the argument name and value
			const std::string arg_name = argument_string.substr(0, equals_char_offset);
			const std::string arg_value = argument_string.substr(equals_char_offset + 1);

			m_command_line_args.emplace(arg_name, arg_value);
		}

		bool init_engine_application(int /*argc*/, char* argv[])
		{
			// Prepare app config
			// TODO: move to a dedicated subsystem!
			VadonApp::Core::Configuration app_configuration;

			// Prepare app config
			{
				app_configuration.app_config.program_name = argv[0];
			}

			// Prepare engine config
			{
				app_configuration.engine_config.core_config.program_name = argv[0];
			}

			// Prepare UI config
			{
				// TODO
			}

			m_engine_app = VadonApp::Core::Application::create_instance();
			return m_engine_app->initialize(app_configuration);
		}

		void post_init()
		{
			const Vadon::Core::Project& project_info = m_game_model.get_project_info();
			if (project_info.startup_scene.is_valid() == true)
			{
				m_game_model.load_level(Model::GameModel::LevelConfiguration{ .scene_id = project_info.startup_scene });
			}
		}

		int execute(int argc, char* argv[])
		{
			if (initialize(argc, argv) == false)
			{
				Vadon::Core::Logger::log_error("Vadon Demo app failed to initialize!\n");
				shutdown();
				return 1;
			}

			post_init();

			while (m_shutdown_requested == false)
			{
				TimePoint current_time = Clock::now();

				m_delta_time = std::chrono::duration_cast<Duration>(current_time - m_last_time_point).count();
				m_last_time_point = current_time;

				// First process platform events
				m_platform_interface.update(); 
				
				// Update model and view
				m_game_model.update();
				m_game_view.update();

				// Update the UI
				m_ui_system.update();

				// Render the results
				m_render_system.update();
			}

			shutdown();
			return 0;
		}

		void request_shutdown()
		{
			m_shutdown_requested = true;
		}

		void shutdown()
		{
			// TODO: shut down game-side systems
			m_engine_app->shutdown();

			Vadon::Core::Logger::log_message("Vadon Demo app successfully shut down.\n");
		}

		bool has_command_line_arg(std::string_view name) const
		{
			auto arg_it = m_command_line_args.find(name.data());
			if (arg_it == m_command_line_args.end())
			{
				return false;
			}

			return true;
		}

		std::string get_command_line_arg(std::string_view name) const
		{
			auto arg_it = m_command_line_args.find(name.data());
			if (arg_it == m_command_line_args.end())
			{
				return std::string();
			}

			return arg_it->second;
		}
	};

	GameCore::GameCore(Vadon::Core::EngineEnvironment& environment)
		: m_internal(std::make_unique<Internal>(*this, environment))
	{
		VadonApp::Core::Application::init_application_environment(environment);
	}

	GameCore::~GameCore() = default;

	int GameCore::execute(int argc, char* argv[])
	{
		return m_internal->execute(argc, argv);
	}

	VadonApp::Core::Application& GameCore::get_engine_app()
	{ 
		return *m_internal->m_engine_app;
	}

	Platform::PlatformInterface& GameCore::get_platform_interface()
	{
		return m_internal->m_platform_interface;
	}

	Render::RenderSystem& GameCore::get_render_system()
	{
		return m_internal->m_render_system;
	}
	
	UI::UISystem& GameCore::get_ui_system()
	{
		return m_internal->m_ui_system;
	}

	float GameCore::get_delta_time() const
	{
		return m_internal->m_delta_time;
	}

	Model::GameModel& GameCore::get_model()
	{
		return m_internal->m_game_model;
	}

	View::GameView& GameCore::get_view()
	{
		return m_internal->m_game_view;
	}

	Vadon::ECS::World& GameCore::get_ecs_world()
	{
		return m_internal->m_ecs_world;
	}

	bool GameCore::has_command_line_arg(std::string_view name) const
	{
		return m_internal->has_command_line_arg(name);
	}

	std::string GameCore::get_command_line_arg(std::string_view name) const
	{
		return m_internal->get_command_line_arg(name);
	}

	void GameCore::request_shutdown()
	{
		m_internal->request_shutdown();
	}
}