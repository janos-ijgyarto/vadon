#include <VadonEditor/Core/Application.hpp>

#include <VadonEditor/Core/CommandLine.hpp>
#include <VadonEditor/Core/Configuration.hpp>
#include <VadonEditor/Core/Logger.hpp>
#include <VadonEditor/Core/Plugin/PluginManager.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Network/NetworkSystem.hpp>

#include <VadonEditor/Simulator/Simulator.hpp>

#include <VadonEditor/UI/UISystem.hpp>

#include <VadonEditor/Network/Message/MessageSerializer.hpp>

#include <QCommandLineParser>
#include <QtWidgets/QApplication>

// TODO: make this always available in debug builds, activated via command line argument!
//#define VADON_EDITOR_ENABLE_DEBUGBREAK_ON_INIT
#if defined(VADON_EDITOR_ENABLE_DEBUGBREAK_ON_INIT)
#include <Windows.h>
#include <QThread>
#endif

namespace
{
	VadonEditor::Core::Logger* s_logger_instance = nullptr;
	QtMessageHandler s_original_message_handler = nullptr;

	VadonEditor::Core::CommandLineState parse_command_line(const QCoreApplication& application)
	{
		// Add the main command line options
		QCommandLineParser command_line_parser;
		command_line_parser.setApplicationDescription("Vadon Editor");
		command_line_parser.addHelpOption();
		command_line_parser.addVersionOption();

		QCommandLineOption simulator_option(VadonEditor::Core::CommandLineState::get_parameter_key(VadonEditor::Core::CommandLineParameter::IS_SIMULATOR),
			QCoreApplication::translate("main", "Run as simulator"));

		command_line_parser.addOption(simulator_option);

		QCommandLineOption schema_exporter_option(VadonEditor::Core::CommandLineState::get_parameter_key(VadonEditor::Core::CommandLineParameter::IS_SCHEMA_EXPORTER),
			QCoreApplication::translate("main", "Run as data schema exporter"));

		command_line_parser.addOption(schema_exporter_option);

		QCommandLineOption debug_break_on_init_option(VadonEditor::Core::CommandLineState::get_parameter_key(VadonEditor::Core::CommandLineParameter::DEBUG_BREAK_ON_INIT),
			QCoreApplication::translate("main", "Force app to pause during initialization to allow attaching debugger"));

		command_line_parser.addOption(debug_break_on_init_option);

		QCommandLineOption startup_project_option(VadonEditor::Core::CommandLineState::get_parameter_key(VadonEditor::Core::CommandLineParameter::STARTUP_PROJECT_PATH),
			QCoreApplication::translate("main", "Path to project to load on startup"),
			QCoreApplication::translate("main", "path"));

		command_line_parser.addOption(startup_project_option);

		command_line_parser.process(application);

		VadonEditor::Core::CommandLineState command_line_state;
		command_line_state.is_simulator = command_line_parser.isSet(simulator_option);
		command_line_state.is_schema_exporter = command_line_parser.isSet(schema_exporter_option);
		command_line_state.debug_break_on_init = command_line_parser.isSet(debug_break_on_init_option);
		command_line_state.startup_project_path = command_line_parser.value(startup_project_option);

		return command_line_state;
	}

	VadonEditor::Core::Configuration get_app_configuration(const VadonEditor::Core::CommandLineState& command_line_state)
	{
		VadonEditor::Core::Configuration configuration;

		if (command_line_state.is_simulator == true)
		{
			configuration.mode = VadonEditor::Core::ApplicationMode::SIMULATOR;
		}

		if (command_line_state.is_schema_exporter == true)
		{
			configuration.mode = VadonEditor::Core::ApplicationMode::SCHEMA_EXPORTER;
		}

		configuration.startup_project_path = command_line_state.startup_project_path;

		return configuration;
	}
}

namespace VadonEditor::Core
{
	struct Application::Internal
	{
		QScopedPointer<QCoreApplication> m_qt_application;
		Configuration m_configuration;

		PluginManager m_plugin_manager;
		ProjectManager m_project_manager;

		Core::Logger m_logger;

		Network::NetworkSystem m_network_system;

		Simulator::Simulator m_simulator;

		UI::UISystem m_ui_system;

		static void message_handler(QtMsgType type, const QMessageLogContext& context, const QString& message)
		{
			s_logger_instance->handle_message(type, context, message);
		}

		Internal(Application& application, QCoreApplication* qt_application, const Configuration& configuration)
			: m_qt_application(qt_application)
			, m_configuration(configuration)
			, m_project_manager(application)
			, m_network_system(application)
			, m_plugin_manager(application)
			, m_simulator(application)
			, m_ui_system(application)
		{
			// Before anything else, we register the message handler
			Q_ASSERT_X(s_logger_instance == nullptr, "Application::Internal", "Logger already initialized");
			s_logger_instance = &m_logger;

			s_original_message_handler = qInstallMessageHandler(&Internal::message_handler);

			// Make sure we can clean up before quitting
			QObject::connect(m_qt_application.data(), &QCoreApplication::aboutToQuit, 
				[this]() 
				{ 
					about_to_quit(); 
				}
			);
		}

		~Internal()
		{
			if (s_logger_instance != nullptr)
			{
				qInstallMessageHandler(s_original_message_handler);
				s_logger_instance = nullptr;
				s_original_message_handler = nullptr;
			}
		}

		bool initialize()
		{
			QObject::connect(&m_project_manager, &ProjectManager::project_loaded,
				[this]()
				{
					project_loaded();
				}
			);

			switch(m_configuration.mode)
			{
			case ApplicationMode::EDITOR:
			{
				QObject::connect(&m_network_system, &Network::NetworkSystem::received_message,
					[&](const QByteArray& data)
					{
						received_message(data);
					}
				);
			}
			break;
			case ApplicationMode::SIMULATOR:
			{
				// On startup, the simulator needs to connect to the editor, then we can load the project
				QObject::connect(&m_network_system, &Network::NetworkSystem::connected_to_server,
					[this]()
					{
						editor_connected();
					}
				);
			}
			break;
			}

			// Initialize UI system first (allows main window to hook into logger)
			if (m_ui_system.initialize() == false)
			{
				return false;
			}

			if (m_project_manager.initialize() == false)
			{
				return false;
			}

			if (m_network_system.initialize() == false)
			{
				return false;
			}

			if (m_simulator.initialize() == false)
			{
				return false;
			}

			return true;
		}

		bool load_startup_project()
		{
			if (m_configuration.mode == ApplicationMode::SIMULATOR)
			{
				// Simulator should run network right away to be able to notify editor server 
				m_network_system.run_network();
			}

			if (m_configuration.startup_project_path.isEmpty() == false)
			{
				qInfo() << "Loading startup project";
				if (m_project_manager.load_project(m_configuration.startup_project_path) == false)
				{
					return false;
				}
			}

			return true;
		}

		void editor_connected()
		{
			if (m_configuration.mode == ApplicationMode::SIMULATOR)
			{
				// Simulator is connected to editor, now we can run the plugin
				Simulator::SimulatorSettings settings; // TODO: apply relevant settings?
				if (m_simulator.run_simulator(settings) == false)
				{
					// Failed to load plugin, exit!
					request_quit(1);
				}
			}
		}

		void cleanup()
		{
			m_simulator.shutdown();
			m_network_system.shutdown();
			m_project_manager.shutdown();

			m_ui_system.shutdown();
		}

		void about_to_quit()
		{
			cleanup();
		}

		void request_quit(int exit_code)
		{
			QMetaObject::invokeMethod(m_qt_application.data(), &QCoreApplication::exit, Qt::QueuedConnection, exit_code);
		}

		void project_loaded()
		{
			switch (m_configuration.mode)
			{
			case ApplicationMode::EDITOR:
			{
				// Show main window
				m_ui_system.show_main_window();
			}
				break;
			case ApplicationMode::SCHEMA_EXPORTER:
			{
				if (m_project_manager.generate_project_data_schema() == true)
				{
					request_quit(0);
				}
				else
				{
					request_quit(1);
				}
				return;
			}
			}

			// Start network to allow communicating with plugins and other tools
			m_network_system.run_network();
		}

		void received_message(const QByteArray& data)
		{
			::Vadon::Foundation::EditorMessageReader message_reader(data.constData(), data.size());

			const ::Vadon::Foundation::EditorMessageCategory category = message_reader.get_current_category();
			switch (category)
			{
			case ::Vadon::Foundation::EditorMessageCategory::TEST:
			{
				const ::Vadon::Foundation::EditorMessageTest* client_test = reinterpret_cast<const ::Vadon::Foundation::EditorMessageTest*>(message_reader.get_current_message_data());

				qInfo() << "Client test message: number = " << client_test->number << ", other number = " << client_test->other_number;
			}
			break;
			}
		}
	};

	Application::Application(int argc, char* argv[])
	{
		// Create Qt core application to allow parsing command line args
		QCoreApplication* qt_application = new QCoreApplication(argc, argv);
		const CommandLineState command_line_state = parse_command_line(*qt_application);

#if defined(VADON_EDITOR_ENABLE_DEBUGBREAK_ON_INIT)
		if (command_line_state.debug_break_on_init == true)
		{
			// TODO: make this properly platform-independent!
			while (IsDebuggerPresent() == 0)
			{
				QThread::sleep(std::chrono::seconds{ 1 });
			}
		}
#endif

		const Configuration app_configuration = get_app_configuration(command_line_state);

		if (app_configuration.mode == ApplicationMode::EDITOR)
		{
			// Replace with Widgets Application
			delete qt_application;
			qt_application = new QApplication(argc, argv);
		}

		QCoreApplication::setOrganizationName(c_org_name);
		QCoreApplication::setApplicationName(c_app_name);

		m_internal = std::make_unique<Internal>(*this, qt_application, app_configuration);
	}

	Application::~Application()
	{
	}

	int Application::exec()
	{
		if (initialize() == false)
		{
			return -1;
		}

		if (m_internal->load_startup_project() == false)
		{
			return -2;
		}

		return m_internal->m_qt_application->exec();
	}

	const Configuration& Application::get_configuration() const
	{
		return m_internal->m_configuration;
	}

	Logger& Application::get_logger()
	{
		return m_internal->m_logger;
	}

	PluginManager& Application::get_plugin_manager()
	{
		return m_internal->m_plugin_manager;
	}

	ProjectManager& Application::get_project_manager()
	{
		return m_internal->m_project_manager;
	}

	Network::NetworkSystem& Application::get_network_system()
	{
		return m_internal->m_network_system;
	}

	Simulator::Simulator& Application::get_simulator()
	{
		return m_internal->m_simulator;
	}

	UI::UISystem& Application::get_ui_system()
	{
		return m_internal->m_ui_system;
	}

	QCoreApplication* Application::get_qt_application() const
	{
		return m_internal->m_qt_application.data();
	}

	void Application::request_quit(int exit_code)
	{
		m_internal->request_quit(exit_code);
	}

	bool Application::initialize()
	{
		return m_internal->initialize();
	}
}