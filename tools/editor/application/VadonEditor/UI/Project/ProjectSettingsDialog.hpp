#ifndef VADONEDITOR_UI_PROJECT_PROJECTSETTINGSDIALOG_HPP
#define VADONEDITOR_UI_PROJECT_PROJECTSETTINGSDIALOG_HPP
#include <QDialog>
#include <VadonEditor/UI/Forms/ui_ProjectSettingsDialog.h>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::UI
{
	class ProjectSettingsDialog : public QDialog
	{
		Q_OBJECT
	public:
		ProjectSettingsDialog(Core::Application& application, QWidget* parent);

		void accept() override;
	private slots:
		void plugin_path_changed(const QString& text);
		void plugin_path_browse_clicked();
	private:
		Core::Application& m_application;
		Ui::ProjectSettings m_ui;

		bool m_plugin_path_changed;
	};
}
#endif