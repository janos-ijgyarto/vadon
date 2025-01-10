#include <VadonDemo/Core/GameCore.hpp>

#include <VadonDemo/Model/Model.hpp>
#include <VadonDemo/Platform/PlatformInterface.hpp>
#include <VadonDemo/Render/RenderSystem.hpp>
#include <VadonDemo/UI/MainWindow.hpp>
#include <VadonDemo/View/View.hpp>

#include <VadonApp/Core/Application.hpp>

#include <VadonApp/Platform/PlatformInterface.hpp>

#include <VadonApp/UI/UISystem.hpp>
#include <VadonApp/UI/Console.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/Core/Environment.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Utilities/Data/Visitor.hpp>

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
		std::unique_ptr<Model::Model> m_model;
		std::unique_ptr<View::View> m_view;
		Platform::PlatformInterface m_platform_interface;
		Render::RenderSystem m_render_system;
		UI::MainWindow m_main_window; // FIXME: should use a UI system instead!
		VadonApp::Core::Application::Instance m_engine_app;

		Vadon::ECS::World m_ecs_world;

		// FIXME: implement a proper CLI parser!
		std::string m_program_name;
		std::unordered_map<std::string, std::string> m_command_line_args;

		TimePoint m_last_time_point;
		float m_delta_time = 0.0f;

		bool m_shutdown_requested = false;

		Internal(GameCore& game_core)
			: m_platform_interface(game_core)
			, m_render_system(game_core)
			, m_main_window(game_core)
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

			// FIXME: need to do this because app and engine only start to exist after initialization
			// Might want to be able to decouple this?
			m_model = std::make_unique<Model::Model>(m_engine_app->get_engine_core());
			if (m_model->initialize() != true)
			{
				return false;
			}

			m_view = std::make_unique<View::View>(m_engine_app->get_engine_core());
			if (m_view->initialize() != true)
			{
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

			if (!m_main_window.initialize())
			{
				return false;
			}

			register_app_event_handlers();

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

		void register_app_event_handlers()
		{
			// Register callback in platform interface
			// FIXME: do this from subsystem!
			m_engine_app->get_system<VadonApp::Platform::PlatformInterface>().register_event_callback(
				[this](const VadonApp::Platform::PlatformEventList& platform_events)
				{
					auto platform_event_visitor = Vadon::Utilities::VisitorOverloadList{
						[this](const VadonApp::Platform::QuitEvent&)
						{
							// Platform is trying to quit, so we request stop
							request_shutdown();
						},
						[this](const VadonApp::Platform::KeyboardEvent& keyboard_event)
						{
							if (keyboard_event.key == VadonApp::Platform::KeyCode::BACKQUOTE)
							{
								m_main_window.show_dev_gui();
							}
							else if (keyboard_event.key == VadonApp::Platform::KeyCode::RETURN)
							{
								if (keyboard_event.down == false && Vadon::Utilities::to_bool(keyboard_event.modifiers & VadonApp::Platform::KeyModifiers::LEFT_ALT))
								{
									m_platform_interface.toggle_fullscreen();
								}
							}
						},
						[](auto) { /* Default, do nothing */ }
					};

					// FIXME: make this more concise using std::visit?
					for (const VadonApp::Platform::PlatformEvent& current_event : platform_events)
					{
						std::visit(platform_event_visitor, current_event);
					}
				}
			);

			// Handle console events
			// TODO: dispatch console events properly!
			{
				VadonApp::UI::Console& console = m_engine_app->get_system<VadonApp::UI::UISystem>().get_console();
				console.register_event_handler(
					[this, &console](const VadonApp::UI::ConsoleCommandEvent& command_event)
					{
						// TODO: parse command
						console.log_error(std::format("Command not recognized: \"{}\"\n", command_event.text));
						return true;
					}
				);
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

			while (m_shutdown_requested == false)
			{
				TimePoint current_time = Clock::now();

				m_delta_time = std::chrono::duration_cast<Duration>(current_time - m_last_time_point).count();
				m_last_time_point = current_time;

				// First process platform events
				m_platform_interface.update(); 
				
				m_main_window.update();
				m_main_window.render();
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
		: m_internal(std::make_unique<Internal>(*this))
	{
		VadonApp::Core::Application::init_application_environment(environment);
		VadonDemo::Model::Model::init_engine_environment(environment);
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

	UI::MainWindow& GameCore::get_main_window()
	{
		return m_internal->m_main_window;
	}

	float GameCore::get_delta_time() const
	{
		return m_internal->m_delta_time;
	}

	Model::Model& GameCore::get_model()
	{
		return *m_internal->m_model;
	}

	View::View& GameCore::get_view()
	{
		return *m_internal->m_view;
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