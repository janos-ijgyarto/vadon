#ifndef VADONEDITOR_UI_MODEL_PROPERTY_TEXT_HPP
#define VADONEDITOR_UI_MODEL_PROPERTY_TEXT_HPP
#include <VadonEditor/UI/Model/Property/Property.hpp>
#include <VadonEditor/UI/Model/Property/ui_PropertyLineEdit.h>
#include <VadonEditor/UI/Model/Property/ui_PropertyPlainTextEdit.h>
namespace VadonEditor::UI
{
	class PropertyLineEdit : public PropertyWidget
	{
		Q_OBJECT
	public:
		PropertyLineEdit(const QUuid& id, const QString& value, QWidget* parent);

		void set_read_only(bool read_only) override;
	private slots:
		void text_changed(const QString& text);
	private:
		Ui::PropertyLineEdit m_ui;
	};

	class PropertyPlainTextEdit : public PropertyWidget
	{
		Q_OBJECT
	public:
		PropertyPlainTextEdit(const QUuid& id, const QString& value, QWidget* parent);

		void set_read_only(bool read_only) override;
	private slots:
		void text_changed();
	private:
		Ui::PropertyPlainTextEdit m_ui;
	};
}
#endif