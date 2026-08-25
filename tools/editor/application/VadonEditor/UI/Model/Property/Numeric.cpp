#include <VadonEditor/UI/Model/Property/Numeric.hpp>

namespace VadonEditor::UI
{
	PropertySpinBox::PropertySpinBox(const QUuid& id, int value, bool is_signed, QWidget* parent)
		: PropertyWidget(id, value, parent)
	{
		m_ui.setupUi(this);

		m_ui.spinBox->setMinimum(is_signed ? INT_MIN : 0);
		m_ui.spinBox->setMaximum(INT_MAX); // FIXME: allow UINT MAX?

		m_ui.spinBox->setValue(value);
	}

	void PropertySpinBox::set_read_only(bool read_only)
	{
		m_ui.spinBox->setReadOnly(read_only);
	}

	void PropertySpinBox::value_changed(int value)
	{
		internal_set_value(value);
	}

	PropertyDoubleSpinBox::PropertyDoubleSpinBox(const QUuid& id, double value, QWidget* parent)
		: PropertyWidget(id, value, parent)
	{
		m_ui.setupUi(this);

		m_ui.doubleSpinBox->setMinimum(-1 * std::numeric_limits<double>::max());
		m_ui.doubleSpinBox->setMaximum(std::numeric_limits<double>::max());

		m_ui.doubleSpinBox->setValue(value);
	}

	void PropertyDoubleSpinBox::set_read_only(bool read_only)
	{
		m_ui.doubleSpinBox->setReadOnly(read_only);
	}

	void PropertyDoubleSpinBox::value_changed(double value)
	{
		internal_set_value(static_cast<float>(value));
	}
}