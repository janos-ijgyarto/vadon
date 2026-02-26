#include <VadonEditor/UI/Model/Property/Property.hpp>

namespace VadonEditor::UI
{
	PropertyListEntry::PropertyListEntry(QWidget* parent, PropertyWidget* property_widget)
		: QWidget(parent)
	{
		m_ui.setupUi(this);

		QHBoxLayout* hbox_layout = new QHBoxLayout();
		hbox_layout->addWidget(property_widget);

		m_ui.frame->setLayout(hbox_layout);
	}
}