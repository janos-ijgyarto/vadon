#ifndef VADONEDITOR_UI_PROJECT_LAUNCHERDIALOG_HPP
#define VADONEDITOR_UI_PROJECT_LAUNCHERDIALOG_HPP
#include <QDialog>
#include <VadonEditor/Forms/ui_LauncherDialog.h>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::UI
{
	class NewProjectDialog;

	class LauncherDialog : public QDialog
	{
		Q_OBJECT
	private slots:
		void new_clicked();
		void import_clicked();

		void run_clicked();
		void remove_clicked();

		void project_selection_changed();
		void project_double_clicked(QListWidgetItem* item);
	private slots:
		void new_project_created();
		void new_project_dialog_destroyed() { m_new_project_dialog = nullptr; }
	private:
		LauncherDialog(Core::Application& application, QWidget* parent = nullptr);

		void init_ui();

		QListWidgetItem* get_selected_item() const;
		void run_selected_project();

		Core::Application& m_application;
		Ui::LauncherDialog m_ui;

		NewProjectDialog* m_new_project_dialog;
		QList<QString> m_projects;

		friend class UISystem;
	};
}
#endif