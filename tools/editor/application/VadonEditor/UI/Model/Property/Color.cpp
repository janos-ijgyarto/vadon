#include <VadonEditor/UI/Model/Property/Color.hpp>

#include <QColor>
#include <QColorDialog>

namespace VadonEditor::UI
{
	PropertyColorRGBA::PropertyColorRGBA(const QUuid& id, const QColor& value, QWidget* parent)
		: PropertyWidget(id, value, parent)
	{
		m_ui.setupUi(this);

		internal_set_color(value);
	}

	void PropertyColorRGBA::set_read_only(bool read_only)
	{
		m_ui.colorPickerButton->setEnabled(read_only == false);
	}

	void PropertyColorRGBA::color_picker_button_clicked()
	{
		const QColor new_color = QColorDialog::getColor(get_value().value<QColor>(), this);
		if (new_color.isValid())
		{
			internal_set_color(new_color);
			internal_set_value(new_color);
		}
	}

	void PropertyColorRGBA::internal_set_color(const QColor& value)
	{
		// TODO: put label inside button, make sure color contrasts 
		// TODO2: can also subclass button and override paintEvent
		m_ui.colorHexLabel->setText(value.name());
		m_ui.colorPickerButton->setStyleSheet(QString("background-color: %1").arg(value.name()));
	}
}