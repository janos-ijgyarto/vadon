#ifndef VADONEDITOR_UI_PROJECT_NEWPROJECTDIALOG_HPP
#define VADONEDITOR_UI_PROJECT_NEWPROJECTDIALOG_HPP
#include <QDialog>
#include <VadonEditor/UI/Project/ui_NewProjectDialog.h>
namespace VadonEditor::Core
{
	struct SourceProjectInfo;
}
namespace VadonEditor::UI
{
	class NewProjectDialog : public QDialog
	{
		Q_OBJECT
	public:
		NewProjectDialog(const Core::SourceProjectInfo& source_info, QWidget* parent);

		Core::SourceProjectInfo get_source_info() const;
		QString get_output_path() const { return m_ui.outputPathLineEdit->text(); }
	private slots:
		void name_edited(const QString& text);

		void root_path_changed(const QString& text);
		void root_path_browse_clicked();

		void output_path_changed(const QString& text);
		void output_path_browse_clicked();
	private:
		void validate_state();

		Ui::NewProjectDialog m_ui;
	};
}
#endif