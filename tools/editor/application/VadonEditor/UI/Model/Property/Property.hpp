#ifndef VADONEDITOR_UI_MODEL_PROPERTY_PROPERTY_HPP
#define VADONEDITOR_UI_MODEL_PROPERTY_PROPERTY_HPP
#include <QWidget>
#include <VadonEditor/UI/Model/Property/ui_PropertyListEntry.h>
namespace VadonEditor::UI
{
	class PropertyWidget : public QWidget
	{
		Q_OBJECT
	public:
		const QVariant& get_value() const { return m_value; }
	signals:
		void value_changed();
	protected:
		PropertyWidget(QWidget* parent) : QWidget(parent) {}
	private:
		QVariant m_value;
	};

	class PropertyListEntry : public QWidget
	{
		Q_OBJECT
	public:
		PropertyListEntry(QWidget* parent, PropertyWidget* property_widget);
	private:
		Ui::PropertyListEntry m_ui;
	};
}
#endif