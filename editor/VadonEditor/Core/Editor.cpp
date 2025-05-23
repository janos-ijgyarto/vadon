#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Platform/PlatformInterface.hpp>
#include <VadonEditor/Render/RenderSystem.hpp>
#include <VadonEditor/UI/UISystem.hpp>
#include <VadonEditor/View/ViewSystem.hpp>

#include <VadonApp/Core/Application.hpp>
#include <VadonApp/Platform/PlatformInterface.hpp>

#include <VadonApp/UI/UISystem.hpp>

#include <Vadon/Core/Environment.hpp>

#include <format>
#include <chrono>
#include <thread>

namespace VadonEditor::Core
{
	struct Editor::Internal
	{
		Editor& m_editor;

		ProjectManager m_project_manager;

		Model::ModelSystem m_model_system;
		Platform::PlatformInterface m_platform_interface;
		Render::RenderSystem m_render_system;
		UI::UISystem m_ui_system;
		View::ViewSystem m_view_system;

		VadonApp::Core::Application::Instance m_engine_app;

		bool m_running = true;
		float m_delta_time = 0.0f;

		// FIXME: implement a proper CLI parser!
		std::string m_program_name;
		std::unordered_map<std::string, std::string> m_command_line_args;

		Internal(Editor& editor)
			: m_editor(editor)
			, m_project_manager(editor)
			, m_model_system(editor)
			, m_platform_interface(editor)
			, m_render_system(editor)
			, m_ui_system(editor)
			, m_view_system(editor)
		{
		}

		bool initialize(int argc, char* argv[])
		{
			// FIXME: implement a proper CLI parser!
			parse_command_line(argc, argv);

			if (init_engine_application() == false)
			{
				Vadon::Core::Logger::log_message("Failed to initialize engine application!\n");
				return false;
			}

			if (m_project_manager.initialize() == false)
			{
				return false;
			}

			if (m_platform_interface.initialize() == false)
			{
				return false;
			}

			if (m_render_system.initialize() == false)
			{
				return false;
			}

			if (m_ui_system.initialize() == false)
			{
				return false;
			}

			if (m_view_system.initialize() == false)
			{
				return false;
			}

			if (m_model_system.initialize() == false)
			{
				return false;
			}

			register_app_event_handlers();

			Vadon::Core::Logger::log_message("Vadon Editor initialized.\n");
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

		bool init_engine_application()
		{
			// Prepare app config
			// TODO: move to a dedicated subsystem!
			VadonApp::Core::Configuration VadonApp_config;

			// Prepare app config
			{
				VadonApp_config.app_config.program_name = m_program_name;
			}

			// Prepare engine config
			{
				VadonApp_config.engine_config.core_config.program_name = m_program_name;
			}

			m_engine_app = VadonApp::Core::Application::create_instance();
			return m_engine_app->initialize(VadonApp_config);
		}

		void register_app_event_handlers()
		{
			// Register callback in platform interface
			m_engine_app->get_system<VadonApp::Platform::PlatformInterface>().register_event_callback(
				[this](const VadonApp::Platform::PlatformEventList& platform_events)
				{
					// FIXME: make this more concise using std::visit?
					for (const VadonApp::Platform::PlatformEvent& current_event : platform_events)
					{
						const VadonApp::Platform::PlatformEventType current_event_type = Vadon::Utilities::to_enum<VadonApp::Platform::PlatformEventType>(static_cast<int32_t>(current_event.index()));
						switch (current_event_type)
						{
						case VadonApp::Platform::PlatformEventType::QUIT:
						{
							// Platform is trying to quit, so we request stop
							m_running = false;
						}
						break;
						}
					}
				}
			);
		}

		int execute()
		{
			using Clock = std::chrono::steady_clock;
			using TimePoint = std::chrono::time_point<Clock>;
			using Duration = std::chrono::duration<float>;

			TimePoint last_frame_time = Clock::now();

			while (m_running == true)
			{
				TimePoint current_time = Clock::now();
				m_delta_time = std::chrono::duration_cast<Duration>(current_time - last_frame_time).count();

				if (m_delta_time < (1.0f / 60.0f))
				{
					std::this_thread::yield();
					continue;
				}

				last_frame_time = current_time;

				// TODO: update engine?

				// Dispatch platform events
				m_platform_interface.update();

				// Check project state
				switch (m_project_manager.get_state())
				{
				case ProjectManager::State::PROJECT_LOADED:
				{
					if (m_model_system.load_project() == true)
					{
						if (m_editor.project_loaded() == true)
						{
							m_project_manager.m_state = ProjectManager::State::PROJECT_OPEN;
						}
						else
						{
							// TODO: handle error!
						}
					}
					else
					{
						// TODO: handle error!
					}
				}
					break;
				case ProjectManager::State::PROJECT_OPEN:
					// Run client update
					m_editor.update();
					break;
				case ProjectManager::State::PROJECT_CLOSED:
					m_running = false;
					break;
				}

				// Update view
				m_view_system.update();

				// Update UI
				m_ui_system.update();

				// Finally we render the current frame
				m_render_system.update();
			}

			shutdown();
			return 0;
		}

		void shutdown()
		{
			m_render_system.shutdown();
			m_engine_app->shutdown();

			Vadon::Core::Logger::log_message("Vadon Editor successfully shut down.\n");
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

	Editor::Editor(Vadon::Core::EngineEnvironment& environment)
		: m_internal(std::make_unique<Internal>(*this))
	{
		VadonApp::Core::Application::init_application_environment(environment);
		Vadon::Core::EngineEnvironment::initialize(environment);
	}

	Editor::~Editor() = default;

	int Editor::execute(int argc, char* argv[])
	{
		if (initialize(argc, argv) == false)
		{
			Vadon::Core::Logger::log_error("Vadon Editor failed to initialize!");
			m_internal->shutdown();
			return 1;
		}

		return m_internal->execute();
	}

	VadonApp::Core::Application& Editor::get_engine_app()
	{
		return *m_internal->m_engine_app;
	}

	Vadon::Core::EngineCoreInterface& Editor::get_engine_core()
	{
		return get_engine_app().get_engine_core();
	}

	float Editor::get_delta_time() const
	{
		return m_internal->m_delta_time;
	}

	bool Editor::has_command_line_arg(std::string_view name) const
	{
		return m_internal->has_command_line_arg(name);
	}

	std::string Editor::get_command_line_arg(std::string_view name) const
	{
		return m_internal->get_command_line_arg(name);
	}
	
	bool Editor::initialize(int argc, char* argv[])
	{
		return m_internal->initialize(argc, argv);
	}
}