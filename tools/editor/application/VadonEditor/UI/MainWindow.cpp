#include <VadonEditor/UI/MainWindow.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Logger.hpp>

#include <VadonEditor/Network/NetworkSystem.hpp>

#include <VadonEditor/UI/Project/ProjectSettingsDialog.hpp>
#include <VadonEditor/UI/Utilities/UUIDDialog.hpp>

#include <VadonEditor/Network/Message/MessageSerializer.hpp>

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
		// TODO!!!
	}

	void MainWindow::shutdown_plugin_triggered()
	{
		// TODO!!!
	}
}