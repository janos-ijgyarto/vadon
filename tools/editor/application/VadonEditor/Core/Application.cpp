#include <VadonEditor/Core/Application.hpp>

#include <VadonEditor/Core/Logger.hpp>
#include <VadonEditor/Core/CommandLine.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Network/NetworkSystem.hpp>

#include <VadonEditor/Simulator/Plugin/PluginManager.hpp>

#include <VadonEditor/UI/UISystem.hpp>

#include <VadonEditor/Network/Message/MessageSerializer.hpp>

#include <Vadon/Foundation/Editor/PluginInterface.hpp>

#include <QCommandLineParser>
#include <QtWidgets/QApplication>

//#define VADON_EDITOR_ENABLE_DEBUGBREAK_ON_INIT
#if defined(VADON_EDITOR_ENABLE_DEBUGBREAK_ON_INIT)
#include <Windows.h>
#include <QThread>
#endif

namespace
{
	VadonEditor::Core::Logger* s_logger_instance = nullptr;
	QtMessageHandler s_original_message_handler = nullptr;

	VadonEditor::Core::CommandLineParameters parse_command_line(const QCoreApplication& application)
	{
		// Add the main command line options
		QCommandLineParser command_line_parser;
		command_line_parser.setApplicationDescription("Vadon Editor");
		command_line_parser.addHelpOption();
		command_line_parser.addVersionOption();

		QCommandLineOption simulator_option("simulator",
			QCoreApplication::translate("main", "Run as simulator"));

		command_line_parser.addOption(simulator_option);

		QCommandLineOption debug_break_on_init_option("debug-break-on-init",
			QCoreApplication::translate("main", "Force app to pause during initialization to allow attaching debugger"));

		command_line_parser.addOption(debug_break_on_init_option);

		QCommandLineOption startup_project_option("startup-project",
			QCoreApplication::translate("main", "Path to project to load on startup"),
			QCoreApplication::translate("main", "path"));

		command_line_parser.addOption(startup_project_option);

		command_line_parser.process(application);

		VadonEditor::Core::CommandLineParameters command_line_params;
		command_line_params.is_simulator = command_line_parser.isSet(simulator_option);
		command_line_params.debug_break_on_init = command_line_parser.isSet(debug_break_on_init_option);
		command_line_params.startup_project_path = command_line_parser.value(startup_project_option);

		return command_line_params;
	}
}

namespace VadonEditor::Core
{
	struct Application::Internal
	{
		QScopedPointer<QCoreApplication> m_qt_application;
		CommandLineParameters m_command_line_params;

		ProjectManager m_project_manager;

		Core::Logger m_logger;

		Network::NetworkSystem m_network_system;

		Simulator::PluginManager m_plugin_manager; // FIXME: only create if we're actually using it!

		UI::UISystem m_ui_system;

		static void message_handler(QtMsgType type, const QMessageLogContext& context, const QString& message)
		{
			s_logger_instance->log_message(message);
			s_original_message_handler(type, context, message);
		}

		Internal(Application& application, QCoreApplication* qt_application, const CommandLineParameters& command_line_params)
			: m_qt_application(qt_application)
			, m_command_line_params(command_line_params)
			, m_project_manager(application)
			, m_network_system(application)
			, m_plugin_manager(application)
			, m_ui_system(application)
		{
			// Before anything else, we register the message handler
			Q_ASSERT_X(s_logger_instance == nullptr, "Application::Internal", "Logger already initialized");
			s_logger_instance = &m_logger;

			s_original_message_handler = qInstallMessageHandler(&Internal::message_handler);

			// Make sure we can clean up before quitting
			QObject::connect(m_qt_application.data(), &QCoreApplication::aboutToQuit, [this]() { about_to_quit(); });
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
#if defined(VADON_EDITOR_ENABLE_DEBUGBREAK_ON_INIT)
			if (m_command_line_params.debug_break_on_init == true)
			{
				// TODO: make this properly platform-independent!
				while (IsDebuggerPresent() == 0)
				{
					QThread::sleep(std::chrono::seconds{ 1 });
				}
			}
#endif
			QObject::connect(&m_project_manager, &ProjectManager::project_loaded,
				[this]()
				{
					project_loaded();
				}
			);

			if (m_command_line_params.is_simulator == false)
			{
				QObject::connect(&m_project_manager, &ProjectManager::project_plugin_path_modified,
					[this]()
					{
						run_simulator();
					}
				);

				QObject::connect(&m_network_system, &Network::NetworkSystem::received_message,
					[&](const QByteArray& data)
					{
						received_message(data);
					}
				);
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

			if (m_plugin_manager.initialize() == false)
			{
				return false;
			}

			if (m_command_line_params.startup_project_path.isEmpty() == false)
			{
				qInfo() << "Loading startup project";
				if (m_project_manager.load_project(m_command_line_params.startup_project_path) == false)
				{
					return false;
				}
			}

			return true;
		}

		void cleanup()
		{
			m_plugin_manager.shutdown();
			m_network_system.shutdown();
			m_project_manager.shutdown();

			m_ui_system.shutdown();
		}

		void about_to_quit()
		{
			cleanup();
		}

		void project_loaded()
		{
			if (m_command_line_params.is_simulator == false)
			{
				// Show main window
				m_ui_system.show_main_window();
			}

			run_simulator();
		}

		void run_simulator()
		{
			const ProjectInfo& project_info = m_project_manager.get_project_info();
			if (project_info.plugin_path.isEmpty() == false)
			{
				// Start the server
				m_network_system.run_network();

				// Run simulator
				m_plugin_manager.run_simulator();
			}
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
		const CommandLineParameters command_line_params = parse_command_line(*qt_application);

		if (command_line_params.is_simulator == false)
		{
			// Replace with Widgets Application
			delete qt_application;
			qt_application = new QApplication(argc, argv);
		}

		QCoreApplication::setOrganizationName(c_org_name);
		QCoreApplication::setApplicationName(c_app_name);

		m_internal = std::make_unique<Internal>(*this, qt_application, command_line_params);
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

		return m_internal->m_qt_application->exec();
	}

	const CommandLineParameters& Application::get_command_line_parameters() const
	{
		return m_internal->m_command_line_params;
	}

	Logger& Application::get_logger()
	{
		return m_internal->m_logger;
	}

	ProjectManager& Application::get_project_manager()
	{
		return m_internal->m_project_manager;
	}

	Network::NetworkSystem& Application::get_network_system()
	{
		return m_internal->m_network_system;
	}

	Simulator::PluginManager& Application::get_plugin_manager()
	{
		return m_internal->m_plugin_manager;
	}

	UI::UISystem& Application::get_ui_system()
	{
		return m_internal->m_ui_system;
	}

	QCoreApplication* VadonEditor::Core::Application::get_qt_application() const
	{
		return m_internal->m_qt_application.data();
	}

	bool Application::initialize()
	{
		return m_internal->initialize();
	}
}