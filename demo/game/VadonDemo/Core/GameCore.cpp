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

#include <Vadon/Utilities/Data/VariantUtilities.hpp>

#include <iostream>
#include <format>

namespace VadonDemo::Core
{
	namespace
	{
		// TODO: implement systems for setting this up based on command line, serialized config, etc.
		constexpr int c_screen_width = 1024;
		constexpr int c_screen_height = 768;
	}

	struct GameCore::Internal
	{
		Vadon::Core::EngineEnvironment m_engine_environment;

		std::unique_ptr<Model::Model> m_model;
		Platform::PlatformInterface m_platform_interface;
		Render::RenderSystem m_render_system;
		UI::MainWindow m_main_window; // FIXME: should use a UI system instead!
		VadonApp::Core::Application::Instance m_engine_app;

		Internal(GameCore& game_core)
			: m_platform_interface(game_core)
			, m_render_system(game_core)
			, m_main_window(game_core)
		{

		}

		bool initialize(int argc, char* argv[])
		{
			if (init_engine_application(argc, argv) == false)
			{
				std::cout << "Failed to initialize engine application!" << std::endl;
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

			auto& console = m_engine_app->get_system<VadonApp::UI::UISystem>().get_console();
			console.log("Vadon Demo app initialized.\n");

			return true;
		}

		bool init_engine_application(int /*argc*/, char* argv[])
		{
			VadonApp::Core::Application::init_application_environment(m_engine_environment);
			VadonDemo::Model::Model::init_engine_environment(m_engine_environment);

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

			// Prepare platform config
			{
				VadonApp::Platform::WindowInfo& main_window_info = app_configuration.platform_config.main_window_info;
				main_window_info.title = "Vadon Demo"; // TODO: version numbering?
				main_window_info.position = Vadon::Utilities::Vector2i(-1, -1);
				main_window_info.size = Vadon::Utilities::Vector2i(c_screen_width, c_screen_height);
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
			m_platform_interface.register_event_callback(
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
						console.error(std::format("Command not recognized: \"{}\"\n", command_event.text));
						return true;
					}
				);
			}
		}

		int execute(int argc, char* argv[])
		{
			if (initialize(argc, argv) == false)
			{
				std::cout << "Vadon Demo app failed to initialize!" << std::endl;
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

			VadonApp::Platform::PlatformInterface& platform_interface = m_engine_app->get_system<VadonApp::Platform::PlatformInterface>();

			// Start main task loop
			game_loop(task_system, platform_interface);

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

			// Keep looping until the task system tells us to stop
			while (!task_system.stop_requested())
			{
				// Use main thread to process platform events
				// NOTE: this may be only a Windows-specific requirement
				m_platform_interface.update();

				// Try to steal some tasks as well
				task_system.consume_task();
			}
		}

		void game_loop(Vadon::Core::TaskSystem& task_system, VadonApp::Platform::PlatformInterface& platform_interface)
		{
			// Check whether we need to exit
			if (task_system.stop_requested())
			{
				return;
			}

			// Update main window once we have the platform events
			Vadon::Core::TaskGroup main_window_group = m_main_window.update();

			// Create a recursive task that will re-run this same function
			Vadon::Core::TaskNode recursive_main_loop_node = main_window_group->create_end_dependent("Vadondemo_recursive_main_loop");
			recursive_main_loop_node->add_subtask(
				[this, &task_system, &platform_interface]()
				{
					game_loop(task_system, platform_interface);
				}
			);

			// Run the main window node (will kick everything else afterward)
			main_window_group->update();
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

			std::cout << "Vadon Demo app successfully shut down." << std::endl;
		}
	};

	GameCore::GameCore()
		: m_internal(std::make_unique<Internal>(*this))
	{
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
}