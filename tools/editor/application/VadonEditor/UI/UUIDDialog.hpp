#ifndef VADONEDITOR_UI_UUIDDIALOG_HPP
#define VADONEDITOR_UI_UUIDDIALOG_HPP
#include <QDialog>
#include <VadonEditor/Forms/ui_UUIDDialog.h>
namespace VadonEditor::UI
{
	class UUIDDialog : public QDialog
	{
		Q_OBJECT
	public:
		UUIDDialog(QWidget* parent);
	private slots:
		void generate_clicked();
	private:
		Ui::UUIDDialog m_ui;
	};
}
#endif