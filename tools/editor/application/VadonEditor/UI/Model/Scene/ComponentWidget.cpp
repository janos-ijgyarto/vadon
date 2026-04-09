#include <VadonEditor/UI/Model/Scene/ComponentWidget.hpp>

namespace VadonEditor::UI
{
	ComponentWidget::ComponentWidget(QWidget* parent)
		: QWidget(parent)
	{
		m_ui.setupUi(this);
	}
}