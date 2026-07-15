#ifndef VADONEDITOR_UI_UTILITIES_CUSTOMMESSAGEBOX_HPP
#define VADONEDITOR_UI_UTILITIES_CUSTOMMESSAGEBOX_HPP
#include <QDialog>
#include <VadonEditor/UI/Utilities/ui_CustomMessageBox.h>
namespace VadonEditor::UI
{
	class CustomMessageBox : public QDialog
	{
		Q_OBJECT
	public:
		CustomMessageBox(QWidget* parent = nullptr);
	protected:
		Ui::CustomMessageBox m_ui;
	};
}
#endif