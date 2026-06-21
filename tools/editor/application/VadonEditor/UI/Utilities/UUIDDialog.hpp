#ifndef VADONEDITOR_UI_UTILITIES_UUIDDIALOG_HPP
#define VADONEDITOR_UI_UTILITIES_UUIDDIALOG_HPP
#include <QDialog>
#include <VadonEditor/UI/Utilities/ui_UUIDDialog.h>
namespace VadonEditor::UI
{
	class UUIDDialog : public QDialog
	{
		Q_OBJECT
	public:
		UUIDDialog(QWidget* parent);
	private slots:
		void generate_clicked();
		void uuid_edited(const QString& text);
		void base64_edited(const QString& text);
	private:
		void set_uuid_line_edit(const QUuid& uuid_value);
		void set_base64_line_edit(const QUuid& uuid_value);

		Ui::UUIDDialog m_ui;
	};
}
#endif