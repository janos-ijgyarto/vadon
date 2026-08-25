#include <VadonEditor/UI/Model/Property/Array.hpp>

namespace VadonEditor::UI
{
	PropertyArrayEntry::PropertyArrayEntry(QWidget* parent, PropertyWidget* property_widget)
		: QWidget(parent)
		, m_property_widget(property_widget)
	{
		m_ui.setupUi(this);

		property_widget->setParent(this);

		QHBoxLayout* hbox_layout = new QHBoxLayout();
		hbox_layout->addWidget(property_widget);

		m_ui.entryDataFrame->setLayout(hbox_layout);

		connect(property_widget, &PropertyWidget::value_changed, this, &PropertyArrayEntry::property_value_changed);
	}

	void PropertyArrayEntry::set_read_only(bool read_only)
	{
		m_ui.deleteButton->setEnabled(read_only == false);
		m_ui.deleteButton->setVisible(read_only == false);

		m_property_widget->set_read_only(read_only);
	}

	void PropertyArrayEntry::update_index(int index)
	{
		m_ui.indexLabel->setText(QString::number(index));
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

	PropertyArray::PropertyArray(const PropertyWidgetInfo& info, QWidget* parent, Model::Resource* owner_resource)
		: PropertyWidget(info.property_id, info.init_value, parent)
		, m_info(info)
		, m_owner_resource(owner_resource)
	{
		Q_ASSERT_X(owner_resource != nullptr, "VadonEditor::UI::PropertyArray::PropertyArray", "Owner resource must not be null!");
		m_ui.setupUi(this);

		initialize();
	}

	void PropertyArray::set_read_only(bool read_only)
	{
		for (int item_index = 0; item_index < m_ui.arrayContentsVBox->count(); ++item_index)
		{
			QWidget* current_widget = m_ui.arrayContentsVBox->itemAt(item_index)->widget();
			PropertyArrayEntry* current_array_entry = qobject_cast<PropertyArrayEntry*>(current_widget);
			if (current_array_entry)
			{
				current_array_entry->set_read_only(read_only);
			}
		}
	}

	void PropertyArray::array_entry_value_changed()
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

	void PropertyArray::array_entry_remove_requested()
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
				array_entry_value_changed();
				update_entry_layout();
				return;
			}
		}
	}

	void PropertyArray::add_entry_triggered()
	{
		PropertyWidgetInfo new_entry_info;

		// NOTE: anything nested in this array will still "belong" to this same property
		// This "chain" is only reset by a sub-object
		new_entry_info.property_id = m_info.property_id;
		new_entry_info.type_list = m_info.type_list;

		// Update offset for the contained element
		new_entry_info.type_list_offset = m_info.type_list_offset + 1;

		new_entry_info.init_value = QVariant(); // FIXME: get default value based on type list?

		PropertyWidget* new_entry_widget = PropertyWidget::create_widget(new_entry_info, this, m_owner_resource);
		if (new_entry_widget == nullptr)
		{
			return;
		}

		internal_add_array_entry(new_entry_widget);
		array_entry_value_changed();

		update_entry_layout();
	}

	void PropertyArray::initialize()
	{
		const QVariantList value_list = get_value().toList();

		PropertyWidgetInfo array_entry_info;

		// NOTE: anything nested in this array will still "belong" to this same property
		// This "chain" is only reset by a sub-object
		array_entry_info.property_id = m_info.property_id;

		array_entry_info.type_list = m_info.type_list;

		// Update offset for the contained entry
		array_entry_info.type_list_offset = m_info.type_list_offset + 1;

		for (const QVariant& current_value : value_list)
		{
			array_entry_info.init_value = current_value;

			PropertyWidget* array_entry_widget = PropertyWidget::create_widget(array_entry_info, this, m_owner_resource);
			if (array_entry_widget == nullptr)
			{
				continue;
			}

			internal_add_array_entry(array_entry_widget);
		}

		update_entry_layout();
	}

	void PropertyArray::internal_add_array_entry(PropertyWidget* property_widget)
	{
		// Insert new item just before the spacer
		const int spacer_index = m_ui.arrayContentsVBox->indexOf(m_ui.arrayContentsSpacer);

		PropertyArrayEntry* new_array_entry = new PropertyArrayEntry(this, property_widget);
		connect(new_array_entry, &PropertyArrayEntry::value_changed, this, &PropertyArray::array_entry_value_changed);
		connect(new_array_entry, &PropertyArrayEntry::remove_requested, this, &PropertyArray::array_entry_remove_requested);

		m_ui.arrayContentsVBox->insertWidget(spacer_index, new_array_entry);
	}

	void PropertyArray::update_entry_layout()
	{
		int entry_index = 0;
		for (int item_index = 0; item_index < m_ui.arrayContentsVBox->count(); ++item_index)
		{
			QWidget* current_widget = m_ui.arrayContentsVBox->itemAt(item_index)->widget();
			PropertyArrayEntry* current_array_entry = qobject_cast<PropertyArrayEntry*>(current_widget);
			if (current_array_entry)
			{
				current_array_entry->update_index(entry_index);
				++entry_index;
			}
		}

		setMinimumHeight(qMin(entry_index * 100, 400));
	}
}