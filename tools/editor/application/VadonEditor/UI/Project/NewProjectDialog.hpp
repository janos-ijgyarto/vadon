#ifndef VADONEDITOR_UI_PROJECT_NEWPROJECTDIALOG_HPP
#define VADONEDITOR_UI_PROJECT_NEWPROJECTDIALOG_HPP
#include <QDialog>
#include <VadonEditor/UI/Forms/ui_NewProjectDialog.h>
namespace VadonEditor::Core
{
	struct ProjectInfo;
}
namespace VadonEditor::UI
{
	class NewProjectDialog : public QDialog
	{
		Q_OBJECT
	public:
		NewProjectDialog(QWidget* parent);

		VadonEditor::Core::ProjectInfo get_project_info() const;
	private slots:
		void name_edited(const QString& text);
		void root_path_edited(const QString& text);
		void browse_clicked();
	private:
		void validate_state();

		Ui::NewProjectDialog m_ui;
	};
}
#endif