#ifndef VADONEDITOR_UI_MODEL_PROPERTY_ARRAY_HPP
#define VADONEDITOR_UI_MODEL_PROPERTY_ARRAY_HPP
#include <VadonEditor/UI/Model/Property/Property.hpp>
#include <VadonEditor/UI/Model/Property/ui_PropertyArray.h>
#include <VadonEditor/UI/Model/Property/ui_PropertyArrayEntry.h>
namespace VadonEditor::UI
{
	class PropertyArrayEntry : public QWidget
	{
		Q_OBJECT
	public:
		PropertyArrayEntry(QWidget* parent, PropertyWidget* property_widget);

		PropertyWidget* get_property_widget() const { return m_property_widget; }
	signals:
		void value_changed();
		void remove_requested();
	private slots:
		void property_value_changed(const QUuid& id);
		void delete_clicked();
	private:
		Ui::PropertyArrayEntry m_ui;
		PropertyWidget* m_property_widget;
	};

	class PropertyArray : public PropertyWidget
	{
		Q_OBJECT
	public:
		PropertyArray(const QUuid& id, const QVariantList& value, QWidget* parent, Model::Resource* owner_resource);
	private slots:
		void array_element_value_changed();
		void array_element_remove_requested();
	private:
		Ui::PropertyArray m_ui;

		Model::Resource* m_owner_resource;
	};
}
#endif