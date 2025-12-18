#include <VadonEditor/UI/MainWindow.hpp>

#include <VadonEditor/UI/UUIDDialog.hpp>

namespace VadonEditor::UI
{
	MainWindow::MainWindow(QWidget* parent)
		: QMainWindow(parent)
	{
		m_ui.setupUi(this);

		connect(m_ui.actionNew, &QAction::triggered, this, &MainWindow::menu_test);

		connect(m_ui.actionGenerate_UUID, &QAction::triggered, this, &MainWindow::generate_uuid_triggered);
	}

	MainWindow::~MainWindow()
	{
	}

	void MainWindow::log_message(const QString& message)
	{
		m_ui.console->appendPlainText(message);
	}

	void MainWindow::generate_uuid_triggered()
	{
		UUIDDialog* uuid_dialog = new UUIDDialog(this);
		uuid_dialog->open();
	}
}