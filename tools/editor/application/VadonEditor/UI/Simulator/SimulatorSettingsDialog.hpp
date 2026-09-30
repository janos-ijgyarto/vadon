#ifndef VADONEDITOR_UI_SIMULATOR_SIMULATORSETTINGSDIALOG_HPP
#define VADONEDITOR_UI_SIMULATOR_SIMULATORSETTINGSDIALOG_HPP
#include <VadonEditor/UI/Simulator/ui_SimulatorSettingsDialog.h>
#include <QDialog>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::UI
{
	class SimulatorSettingsDialog : public QDialog
	{
		Q_OBJECT
	public:
		SimulatorSettingsDialog(Core::Application& application, QWidget* parent);
	private slots:
		void setting_value_changed(const QUuid& setting_id);
	private:
		Ui::SimulatorSettingsDialog m_ui;

		Core::Application& m_application;
	};
}
#endif