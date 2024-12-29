#include <VadonDemo/Core/GameCore.hpp>

#include <VadonDemo/Model/Model.hpp>
#include <VadonDemo/Platform/PlatformInterface.hpp>
#include <VadonDemo/Render/RenderSystem.hpp>
#include <VadonDemo/UI/MainWindow.hpp>

#include <VadonApp/Core/Application.hpp>

#include <VadonApp/Platform/PlatformInterface.hpp>

#include <VadonApp/UI/UISystem.hpp>
#include <VadonApp/UI/Console.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/Core/Environment.hpp>
#include <Vadon/Core/Task/TaskSystem.hpp>

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
				app_configuration.ui_config.dev_gui_config.frame_count = 4; // Prepare 4 frames so we can triple buffer and have a ready buffer
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
							Vadon::Core::TaskSystem& task_system = m_engine_app->get_engine_core().get_system<Vadon::Core::TaskSystem>();
							task_system.request_stop();
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

			// Start up the task system
			Vadon::Core::TaskConfiguration task_config;
			task_config.thread_count = 4;

			Vadon::Core::TaskSystem& task_system = m_engine_app->get_engine_core().get_system<Vadon::Core::TaskSystem>();
			
			if (!task_system.start(task_config))
			{
				shutdown();
				return 1;
			}

			// Start render task loop
			render_loop(task_system);

			// Put main thread in a separate loop
			main_thread_loop();

			shutdown();
			return 0;
		}

		void main_thread_loop()
		{
			Vadon::Core::TaskSystem& task_system = m_engine_app->get_engine_core().get_system<Vadon::Core::TaskSystem>();

			bool frame_completed = true;

			// Keep looping until the task system tells us to stop
			while (!task_system.stop_requested())
			{
				if (frame_completed == true)
				{
					TimePoint current_time = Clock::now();

					m_delta_time = std::chrono::duration_cast<Duration>(current_time - m_last_time_point).count();
					m_last_time_point = current_time;

					frame_completed = false;

					// Use main thread to process platform events
					// NOTE: this may be only a Windows-specific requirement
					m_platform_interface.update();

					// Update main window once we have the platform events
					Vadon::Core::TaskGroup main_window_group = m_main_window.update();

					Vadon::Core::TaskNode frame_end_node = main_window_group->create_end_dependent("Vadondemo_frame_end");
					frame_end_node->add_subtask([&frame_completed]()
						{
							frame_completed = true;
						}
					);

					// Run the main window node (will kick everything else afterward)
					main_window_group->update();
				}

				// Try to steal some tasks as well
				task_system.consume_task();
			}
		}

		void render_loop(Vadon::Core::TaskSystem& task_system)
		{
			// Check whether we need to exit
			if (task_system.stop_requested())
			{
				return;
			}

			// Get render data from main window
			Vadon::Core::TaskNode window_render_node = task_system.create_task_node("Vadondemo_main_window_render");
			window_render_node->add_subtask(
				[this]()
				{
					m_main_window.render();
				}
			);

			// Update the render system
			Vadon::Core::TaskNode render_system_node = window_render_node->create_dependent("Vadondemo_render_system");
			render_system_node->add_subtask(
				[this, &task_system]()
				{
					m_render_system.update();
				}
			);

			// Have a looping task that keeps rendering
			Vadon::Core::TaskNode render_loop_node = render_system_node->create_dependent("Vadondemo_render_loop");
			render_loop_node->add_subtask(
				[this, &task_system]()
				{
					// Have task enqueue itself again
					// TODO: limit framerate?
					render_loop(task_system);
				}
			);

			// Kick the window node (will enqueue its dependents afterward)
			window_render_node->update();
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
}