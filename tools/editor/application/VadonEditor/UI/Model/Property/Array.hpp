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

		void set_read_only(bool read_only);

		void update_index(int index);
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
		PropertyArray(const PropertyWidgetInfo& info, QWidget* parent, Model::Resource* owner_resource);

		void set_read_only(bool read_only) override;
	private slots:
		void array_entry_value_changed();
		void array_entry_remove_requested();

		void add_entry_triggered();
	private:
		void initialize();

		void internal_add_array_entry(PropertyWidget* property_widget);
		void update_entry_layout();

		Ui::PropertyArray m_ui;

		PropertyWidgetInfo m_info;
		Model::Resource* m_owner_resource;
	};
}
#endif