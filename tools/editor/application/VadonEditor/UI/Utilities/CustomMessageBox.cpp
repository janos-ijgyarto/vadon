#include <VadonEditor/UI/Utilities/CustomMessageBox.hpp>

namespace VadonEditor::UI
{
	CustomMessageBox::CustomMessageBox(QWidget* parent)
		: QDialog(parent)
	{
		m_ui.setupUi(this);
	}
}