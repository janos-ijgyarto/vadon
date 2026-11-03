#include <VadonEditor/UI/Model/Property/Text.hpp>

#include <QTextBlock>

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

		adjust_height();
	}

	void PropertyPlainTextEdit::set_read_only(bool read_only)
	{
		m_ui.plainTextEdit->setReadOnly(read_only);
	}

	void PropertyPlainTextEdit::text_changed()
	{
		// TODO: add delay?
		internal_set_value(m_ui.plainTextEdit->toPlainText());

		adjust_height();
	}

	void PropertyPlainTextEdit::adjust_height()
	{
		// FIXME: this is still not exactly accurate, but it's good enough for now
		QFontMetrics font_metrics(m_ui.plainTextEdit->font());
		const int line_spacing = font_metrics.lineSpacing();
		const int block_count = m_ui.plainTextEdit->blockCount();
		
		const int total_height = qMax((line_spacing * block_count) + (m_ui.plainTextEdit->frameWidth() * 2) + 4, 30);

		m_ui.plainTextEdit->setFixedHeight(qMin(total_height, 300));
	}
}