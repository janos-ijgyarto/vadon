#include <VadonEditor/UI/MainWindow.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Logger.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Network/NetworkSystem.hpp>

#include <VadonEditor/UI/UISystem.hpp>

#include <VadonEditor/UI/Project/DataSchemaDialog.hpp>
#include <VadonEditor/UI/Project/ProjectSettingsDialog.hpp>

#include <VadonEditor/UI/Utilities/UUIDDialog.hpp>

#include <VadonEditor/Network/Message/MessageSerializer.hpp>

#include <QMessageBox>

namespace VadonEditor::UI
{
	MainWindow::MainWindow(Core::Application& application, QWidget* parent)
		: QMainWindow(parent)
		, m_application(application)
	{
		m_ui.setupUi(this);

		QObject::connect(&m_application.get_logger(), &Core::Logger::message_logged, this, &UI::MainWindow::message_logged);
	}

	MainWindow::~MainWindow()
	{
	}

	void MainWindow::message_logged(const QString& message)
	{
		m_ui.console->appendPlainText(message);
	}

	void MainWindow::new_triggered()
	{
		::Vadon::Foundation::EditorMessageTest test_message;
		test_message.number = 123;
		test_message.other_number = 4.567f;

		VadonEditor::Network::MessageSerializer message_serializer;
		message_serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::TEST, test_message);

		m_application.get_network_system().send_message(message_serializer);
	}

	void MainWindow::quit_triggered()
	{
		close();
	}

	void MainWindow::generate_uuid_triggered()
	{
		UUIDDialog* uuid_dialog = new UUIDDialog(this);
		uuid_dialog->open();
	}

	void MainWindow::project_settings_triggered()
	{
		ProjectSettingsDialog* project_settings_dialog = new ProjectSettingsDialog(m_application, this);
		project_settings_dialog->open();
	}

	void MainWindow::run_plugin_triggered()
	{
		emit run_simulator_requested();
	}

	void MainWindow::shutdown_plugin_triggered()
	{
		emit stop_simulator_requested();
	}

	void MainWindow::project_data_schema_triggered()
	{
		DataSchemaDialog* data_schema_dialog = new DataSchemaDialog(m_application, this);
		data_schema_dialog->open();
	}

	void MainWindow::generate_data_schema_triggered()
	{
		Core::ProjectManager& project_manager = m_application.get_project_manager();
		if (project_manager.generate_project_data_schema() == false)
		{
			QMessageBox::critical(this, "Project manager error", "Failed to generate project data schema!");
		}

		if(project_manager.load_project_data_schema() == false)
		{
			QMessageBox::critical(this, "Project manager error", "Failed to load project data schema!");
		}
	}

	void MainWindow::closeEvent(QCloseEvent* event)
	{
		if (m_application.get_ui_system().is_shutting_down() == false)
		{
			// Emit signal so UI system decides if we are ready to shut down
			emit(close_requested());

			// Ignore event so we don't actually close
			event->ignore();
			return;
		}

		event->accept();
	}
}