#include <VadonEditor/UI/Model/Property/Text.hpp>

namespace VadonEditor::UI
{
	PropertyLineEdit::PropertyLineEdit(const QUuid& id, const QString& value, QWidget* parent)
		: PropertyWidget(id, value, parent)
	{
		m_ui.setupUi(this);
	}

	void PropertyLineEdit::set_read_only(bool read_only)
	{
		m_ui.lineEdit->setReadOnly(read_only);
	}

	void PropertyLineEdit::text_changed(const QString& text)
	{
		// TODO: add delay?
		internal_set_value(text);
	}

	PropertyPlainTextEdit::PropertyPlainTextEdit(const QUuid& id, const QString& value, QWidget* parent)
		: PropertyWidget(id, value, parent)
	{
		m_ui.setupUi(this);

		// Temporarily block signals while we set the value
		m_ui.plainTextEdit->blockSignals(true);
		m_ui.plainTextEdit->setPlainText(value);
		m_ui.plainTextEdit->blockSignals(false);
	}

	void PropertyPlainTextEdit::set_read_only(bool read_only)
	{
		m_ui.plainTextEdit->setReadOnly(read_only);
	}

	void PropertyPlainTextEdit::text_changed()
	{
		// TODO: add delay?
		internal_set_value(m_ui.plainTextEdit->toPlainText());
	}
}