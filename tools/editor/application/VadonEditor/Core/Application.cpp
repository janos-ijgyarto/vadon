#include <VadonEditor/Core/Application.hpp>

#include <VadonEditor/Core/Logger.hpp>
#include <VadonEditor/Core/Settings.hpp>

#include <VadonEditor/Network/MessageSystem.hpp>
#include <VadonEditor/Network/NetworkSystem.hpp>

#include <VadonEditor/Simulator/API/Plugin.hpp>
#include <VadonEditor/Simulator/Plugin/PluginManager.hpp>

#include <VadonEditor/UI/MainWindow.hpp>

#include <VadonEditor/Network/Message/MessageSerializer.hpp>

#include <QCommandLineParser>
#include <QtWidgets/QApplication>
#include <QProcess>
#include <QStyleFactory>
#include <QThread>

#include <QTimer>

#include <iostream>

namespace
{
	VadonEditor::Core::Logger* s_logger_instance = nullptr;
	QtMessageHandler s_original_message_handler = nullptr;

	VadonEditor::Core::EditorSettings parse_command_line(const QCoreApplication& application)
	{
		// Add the main command line options
		QCommandLineParser command_line_parser;
		command_line_parser.setApplicationDescription("Vadon Editor");
		command_line_parser.addHelpOption();
		command_line_parser.addVersionOption();

		QCommandLineOption simulator_option("simulator",
			QCoreApplication::translate("main", "Run as simulator"));

		command_line_parser.addOption(simulator_option);

		QCommandLineOption plugin_path_option("plugin_path",
			QCoreApplication::translate("main", "Override path to plugin DLL"),
			QCoreApplication::translate("main", "path"));

		command_line_parser.addOption(plugin_path_option);

		command_line_parser.process(application);

		VadonEditor::Core::EditorSettings settings;
		settings.is_simulator = command_line_parser.isSet(simulator_option);
		settings.plugin_path = command_line_parser.value(plugin_path_option);

		return settings;
	}
}

namespace VadonEditor::Core
{
	struct Application::Internal
	{
		QScopedPointer<QCoreApplication> m_qt_application;
		EditorSettings m_settings;

		QProcess m_process;
		QThread m_network_thread;

		Core::Logger m_logger;

		Network::NetworkSystem* m_network_system;
		Network::MessageSystem m_message_system;

		Simulator::PluginManager m_plugin_manager; // FIXME: only create if we're actually using it!

		UI::MainWindow* m_main_window;

		static void message_handler(QtMsgType type, const QMessageLogContext& context, const QString& message)
		{
			s_logger_instance->internal_log_message(message);
			s_original_message_handler(type, context, message);
		}

		Internal(Application& application, QCoreApplication* qt_application, const EditorSettings settings)
			: m_qt_application(qt_application)
			, m_settings(settings)
			, m_network_system(nullptr)
			, m_main_window(nullptr)
			, m_plugin_manager(application)
		{
			// Before anything else, we register the message handler
			Q_ASSERT_X(s_logger_instance == nullptr, "Application::Internal", "Logger already initialized");
			s_logger_instance = &m_logger;

			s_original_message_handler = qInstallMessageHandler(&Internal::message_handler);

			// Make sure we can clean up before quitting
			QObject::connect(m_qt_application.data(), &QCoreApplication::aboutToQuit, [this]() { about_to_quit(); });

			// Create network system (will be moved to thread, so we allocate dynamically)
			m_network_system = new Network::NetworkSystem(application);
		}

		bool initialize(Application& application)
		{
			if (m_settings.is_simulator == false)
			{
				// Main GUI application, create the main window
				// Make it early so we can display logs, etc.
				if (init_main_window() == false)
				{
					return false;
				}

				// After 3 seconds, launch the client app
				// FIXME: do this by opening a project!
				QTimer::singleShot(3000, [this]() { test_embed(); });
			}
			else
			{
				// Simulator application
				// TODO: anything specific?
			}

			if (init_network(application) == false)
			{
				return false;
			}
			
			if (m_settings.is_simulator == true)
			{
				// Only initialize the plugin manager if we are in simulator mode
				// FIXME: refactor this!
				if (m_plugin_manager.initialize() == false)
				{
					return false;
				}
			}

			// Start the network thread
			m_network_thread.start();

			return true;
		}

		// FIXME: move this to a "GUISystem" that encapsulates all of this
		bool init_main_window()
		{
			// TODO: initialization function for MainWindow? (e.g using command line args to set certain parameters?)
			QApplication* qt_widgets_app = qobject_cast<QApplication*>(m_qt_application.data());

			// Use fusion style (allows us to add dark mode)
			qt_widgets_app->setStyle(QStyleFactory::create("Fusion"));

			m_main_window = new UI::MainWindow();

			QObject::connect(&m_logger, &Logger::log_message, m_main_window, &UI::MainWindow::log_message);

			QObject::connect(m_main_window, &UI::MainWindow::menu_test,
				[this]()
				{
					VadonEditor::Network::TestMessage test_message;
					test_message.number = 123;
					test_message.other_number = 4.567f;

					std::vector<char> message_data;
					VadonEditor::Network::MessageSerializer::write_message(test_message, message_data);

					QByteArray message_buffer;
					message_buffer.append(message_data.data(), message_data.size());

					m_message_system.send_message(message_buffer);
				}
			);

			m_main_window->show();

			return true;
		}

		// FIXME: move this to a "NetworkSystem" that encapsulates all of this
		bool init_network(Application& application)
		{
			if (m_network_system->initialize() == false)
			{
				return false;
			}

			// TODO: initialization function for Network (make use of command line, etc.)

			m_network_system->moveToThread(&m_network_thread);

			QObject::connect(&m_network_thread, &QThread::started, m_network_system, &Network::NetworkSystem::start);
			QObject::connect(&m_network_thread, &QThread::finished, m_network_system, &Network::NetworkSystem::close);
			QObject::connect(&m_network_thread, &QThread::finished, m_network_system, &QObject::deleteLater);

			QObject::connect(&m_message_system, &Network::MessageSystem::message_dispatched, m_network_system, &Network::NetworkSystem::send_message);

			QObject::connect(m_network_system, &Network::NetworkSystem::received_message, &m_message_system, &Network::MessageSystem::internal_received_message);

			QObject::connect(&m_message_system, &Network::MessageSystem::received_message,
				[&](const QByteArray& data)
				{
					const VadonEditor::Network::MessageHeader* message_header = reinterpret_cast<const VadonEditor::Network::MessageHeader*>(data.data());
					const VadonEditor::Network::MessageCategory category = static_cast<VadonEditor::Network::MessageCategory>(message_header->category);

					switch (category)
					{
					case VadonEditor::Network::MessageCategory::TEST:
					{
						if (m_settings.is_simulator == false)
						{
							VadonEditor::Network::TestMessage client_test;
							VadonEditor::Network::MessageSerializer::deserialize_message(client_test, data.data() + sizeof(VadonEditor::Network::MessageHeader));

							qInfo() << "Client test message: number = " << client_test.number << ", other number = " << client_test.other_number << Qt::endl;
						}
						else
						{
							// Send to plugin
							// FIXME: move to other system!
							application.get_plugin_manager().get_plugin()->process_message_from_editor(*message_header, data.data() + sizeof(VadonEditor::Network::MessageHeader));
						}
					}
					break;
					}
				}
			);

			return true;
		}

		~Internal()
		{
			cleanup();

			Q_ASSERT_X(m_network_system == nullptr, "Application::~Internal", "Network system was not cleaned up");
			Q_ASSERT_X(m_main_window == nullptr, "Application::~Internal", "Main window was not cleaned up");

			if (s_logger_instance != nullptr)
			{
				qInstallMessageHandler(s_original_message_handler);
				s_logger_instance = nullptr;
				s_original_message_handler = nullptr;
			}
		}

		void cleanup()
		{
			m_plugin_manager.shutdown();

			if (m_network_thread.isRunning() == true)
			{
				m_network_thread.quit();
				m_network_thread.wait();

				// The thread will already clean up the network system
				m_network_system = nullptr;
			}

			if (m_network_system != nullptr)
			{
				// In case we never moved it to the network thread
				m_network_system->deleteLater();
				m_network_system = nullptr;
			}

			if (m_main_window != nullptr)
			{
				m_main_window->deleteLater();
				m_main_window = nullptr;
			}
		}

		void test_embed()
		{
			if (m_settings.is_simulator == true)
			{
				return;
			}

			QString program_path = QCoreApplication::applicationFilePath();
			m_process.setProgram(program_path);
			m_process.setArguments({ "--simulator" });

			QObject::connect(&m_process, &QProcess::aboutToClose, [this]() {cleanup_process(); });
			QObject::connect(&m_process, &QProcess::errorOccurred, [this](QProcess::ProcessError error) { process_error(error); });

			QObject::connect(&m_process, &QProcess::readyReadStandardOutput,
				[this]()
				{
					qInfo() << qPrintable(m_process.readAllStandardOutput());
				}
			);

			m_process.start(QIODevice::ReadOnly);
		}

		void cleanup_process()
		{
			qInfo() << "Process shutting down" << Qt::endl;
		}

		void process_error(QProcess::ProcessError error)
		{
			qCritical() << "Error running process: " << error << Qt::endl;
		}

		void about_to_quit()
		{
			cleanup();
		}
	};

	Application::Application(int argc, char* argv[])
	{
		// Create Qt core application to allow parsing command line args
		QCoreApplication* qt_application = new QCoreApplication(argc, argv);
		const EditorSettings& settings = parse_command_line(*qt_application);

		if (settings.is_simulator == false)
		{
			// Replace with Widgets Application
			delete qt_application;
			qt_application = new QApplication(argc, argv);
		}
		else
		{
			QCoreApplication::setApplicationName("Vadon Editor (Simulator)");
		}

		m_internal = std::make_unique<Internal>(*this, qt_application, settings);
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

	const EditorSettings& Application::get_settings() const
	{
		return m_internal->m_settings;
	}

	Network::MessageSystem& Application::get_message_system()
	{
		return m_internal->m_message_system;
	}

	Network::NetworkSystem& Application::get_network_system()
	{
		return *m_internal->m_network_system;
	}

	Simulator::PluginManager& Application::get_plugin_manager()
	{
		return m_internal->m_plugin_manager;
	}

	UI::MainWindow* Application::get_main_window()
	{
		return m_internal->m_main_window;
	}

	bool Application::initialize()
	{
		return m_internal->initialize(*this);
	}
}