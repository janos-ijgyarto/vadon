#include <VadonEditor/UI/UISystem.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Configuration.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Network/NetworkSystem.hpp>
#include <VadonEditor/Network/Message/MessageSerializer.hpp>

#include <VadonEditor/Simulator/Simulator.hpp>

#include <VadonEditor/UI/MainWindow.hpp>
#include <VadonEditor/UI/Project/LauncherDialog.hpp>

#include <Vadon/Foundation/Editor/Network/Message/Platform.hpp>

#include <QMessageBox>
#include <QStyleFactory>

namespace VadonEditor::UI
{
	UISystem::UISystem(Core::Application& application)
		: m_application(application)
		, m_launcher_dialog(nullptr)
		, m_main_window(nullptr)
	{

	}

	bool UISystem::initialize()
	{
		const Core::Configuration& configuration = m_application.get_configuration();
		if (configuration.mode != Core::ApplicationMode::EDITOR)
		{
			// Running as simulator, no GUI
			return true;
		}

		QApplication* qt_widgets_app = qobject_cast<QApplication*>(m_application.get_qt_application());

		// Use fusion style (allows us to add dark mode)
		qt_widgets_app->setStyle(QStyleFactory::create("Fusion"));

		QObject::connect(&m_application.get_network_system(), &Network::NetworkSystem::received_message,
			[&](const QByteArray& data)
			{
				received_message(data);
			}
		);

		m_main_window = new MainWindow(m_application);

		QObject::connect(m_main_window, &MainWindow::run_simulator_requested, [this]() { run_simulator(); });
		QObject::connect(m_main_window, &MainWindow::stop_simulator_requested, [this]() { stop_simulator(); });

		if (configuration.startup_project_path.isEmpty() == true)
		{
			// Open launcher dialog first
			m_launcher_dialog = new LauncherDialog(m_application);
			QObject::connect(m_launcher_dialog, &LauncherDialog::accepted,
				[this]()
				{
					launcher_accepted();
				}
			);
			m_launcher_dialog->open();
		}
		else
		{
			// Startup project, go straight to main window
			show_main_window();
		}

		return true;
	}

	void UISystem::shutdown()
	{
		if (m_launcher_dialog != nullptr)
		{
			m_launcher_dialog->deleteLater();
			m_launcher_dialog = nullptr;
		}

		if (m_main_window != nullptr)
		{
			m_main_window->deleteLater();
			m_main_window = nullptr;
		}
	}

	void UISystem::launcher_accepted()
	{
		// TODO: anything else?

		m_launcher_dialog->deleteLater();
		m_launcher_dialog = nullptr;
	}

	void UISystem::show_main_window()
	{
		m_main_window->show();

		if (m_application.get_project_manager().get_project_data_schema().is_valid() == false)
		{
			QMessageBox::warning(m_main_window, "Project Manager", QObject::tr("Project does not have valid data schema!"));
		}
	}

	void UISystem::request_close()
	{
		// TODO: go through all windows and initiate a closeEvent
		// Windows can check whether we need to prompt the user for confirmation
		// We rely on QApplication to shut down once all top-level windows are closed
	}

	void UISystem::received_message(const QByteArray& data)
	{
		::Vadon::Foundation::EditorMessageReader message_reader(data.constData(), data.size());

		switch (message_reader.get_current_category())
		{
		case ::Vadon::Foundation::EditorMessageCategory::PLATFORM:
		{
			const ::Vadon::Foundation::EditorPlatformMessageHeader* platform_message_header = reinterpret_cast<const ::Vadon::Foundation::EditorPlatformMessageHeader*>(message_reader.get_current_message_data());
			switch (platform_message_header->message_type)
			{
			case ::Vadon::Foundation::EditorPlatformMessageType::MANAGER_WINDOW_REQUEST:
			{
				// Send the main viewport as our response
				// TODO: allow clients to create and register other widgets!
				const ::Vadon::Foundation::EditorPlatformManagerWindowRequest* window_request = reinterpret_cast<const ::Vadon::Foundation::EditorPlatformManagerWindowRequest*>(message_reader.get_current_message_data());

				RenderClientInfo client_info;
				client_info.application = &m_application;
				client_info.client_id = window_request->id;

				RenderWidget* main_viewport = m_main_window->get_viewport();
				main_viewport->register_client(client_info);

				::Vadon::Foundation::EditorPlatformManagerWindowRequest window_request_response;
				window_request_response.message_type = ::Vadon::Foundation::EditorPlatformMessageType::MANAGER_WINDOW_REQUEST;
				window_request_response.id = window_request->id;
				window_request_response.handle = main_viewport->winId();

				VadonEditor::Network::MessageSerializer message_serializer;
				message_serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::PLATFORM, window_request_response);

				m_application.get_network_system().send_message(message_serializer);
			}
			break;
			}
		}
		break;
		}
	}

	void UISystem::run_simulator()
	{
		// TODO: gather settings!
		Simulator::SimulatorSettings settings;
		settings.debug_break_on_init = true;
		if (m_application.get_simulator().run_simulator(settings) == false)
		{
			// TODO: error popup?
		}
	}

	void UISystem::stop_simulator()
	{
		m_application.get_simulator().stop_simulator();
	}
}