#include <VadonEditor/UI/Model/Property/Array.hpp>

namespace VadonEditor::UI
{
	PropertyArrayEntry::PropertyArrayEntry(QWidget* parent, PropertyWidget* property_widget)
		: QWidget(parent)
		, m_property_widget(property_widget)
	{
		m_ui.setupUi(this);

		QHBoxLayout* hbox_layout = new QHBoxLayout();
		hbox_layout->addWidget(property_widget);

		m_ui.entryDataFrame->setLayout(hbox_layout);

		connect(property_widget, &PropertyWidget::value_changed, this, &PropertyArrayEntry::property_value_changed);
	}

	void PropertyArrayEntry::property_value_changed(const QUuid& id)
	{
		Q_UNUSED(id);
		emit(value_changed());
	}

	void PropertyArrayEntry::delete_clicked()
	{
		emit(remove_requested());
	}

	PropertyArray::PropertyArray(const QUuid& id, const QVariantList& value, QWidget* parent, Model::Resource* owner_resource)
		: PropertyWidget(id, value, parent)
		, m_owner_resource(owner_resource)
	{
		Q_ASSERT_X(owner_resource != nullptr, "VadonEditor::UI::PropertyArray::PropertyArray", "Owner resource must not be null!");
		m_ui.setupUi(this);
	}

	void PropertyArray::array_element_value_changed()
	{
		QVariantList values;
		for (int item_index = 0; item_index < m_ui.arrayContentsVBox->count(); ++item_index)
		{
			QWidget* current_widget = m_ui.arrayContentsVBox->itemAt(item_index)->widget();
			PropertyArrayEntry* current_array_entry = qobject_cast<PropertyArrayEntry*>(current_widget);
			if (current_array_entry)
			{
				values.push_back(current_array_entry->get_property_widget()->get_value());
			}
		}

		internal_set_value(values);
	}

	void PropertyArray::array_element_remove_requested()
	{
		QObject* sender_object = QObject::sender();
		QWidget* sender_widget = qobject_cast<QWidget*>(sender_object);

		if (sender_widget == nullptr)
		{
			// TODO: error?
			return;
		}

		for (int item_index = 0; item_index < m_ui.arrayContentsVBox->count(); ++item_index)
		{
			QWidget* current_widget = m_ui.arrayContentsVBox->itemAt(item_index)->widget();
			if (current_widget == sender_widget)
			{
				delete current_widget;
				array_element_value_changed();
				return;
			}
		}
	}
}