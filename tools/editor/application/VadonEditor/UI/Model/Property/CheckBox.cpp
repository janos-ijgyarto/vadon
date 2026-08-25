#include <VadonEditor/UI/Model/Property/CheckBox.hpp>

namespace VadonEditor::UI
{
	PropertyCheckBox::PropertyCheckBox(const QUuid& id, bool value, QWidget* parent)
		: PropertyWidget(id, value, parent)
	{
		m_ui.setupUi(this);

		// NOTE: Designer did not have this function, have to connect manually
		connect(m_ui.checkBox, &QCheckBox::checkStateChanged, this, &PropertyCheckBox::check_state_changed);

		m_ui.checkBox->setCheckState(value ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
	}

	void PropertyCheckBox::check_state_changed(Qt::CheckState state)
	{
		internal_set_value(state == Qt::CheckState::Checked);
	}

	void PropertyCheckBox::set_read_only(bool read_only)
	{
		m_ui.checkBox->setEnabled(read_only == false);
	}
}