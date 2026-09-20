#include <VadonEditor/UI/Model/Object/ObjectEditor.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Data/Object.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/UI/Model/Property/Property.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

namespace VadonEditor::UI
{
	ObjectEditor::ObjectEditor(Core::DataObject& data_object, Model::Resource* owner_resource, QWidget* parent)
		: QWidget(parent)
		, m_object(data_object)
	{
		m_ui.setupUi(this);

		initialize(owner_resource);
	}

	void ObjectEditor::set_read_only(bool read_only)
	{
		for (int item_index = 0; item_index < m_ui.scrollContentsLayout->count(); ++item_index)
		{
			QWidget* current_widget = m_ui.scrollContentsLayout->itemAt(item_index)->widget();
			PropertyListEntry* list_entry = qobject_cast<PropertyListEntry*>(current_widget);
			if (list_entry != nullptr)
			{
				PropertyWidget* property_widget = list_entry->get_property_widget();
				Q_ASSERT_X(property_widget != nullptr, "VadonEditor::UI::ObjectEditor::set_read_only", "Cannot find property widget");
				property_widget->set_read_only(read_only);
			}
		}
	}

	QList<QWidget*> ObjectEditor::generate_property_widgets(const Core::DataObject& data_object, Model::Resource* owner_resource, QWidget* parent_widget)
	{
		QWidgetList widget_list;

		Core::Application& application = data_object.get_application();
		const Core::DataSchema& data_schema = application.get_project_manager().get_project_data_schema();
		const Core::TypeData* type_data = data_schema.find_type_data(data_object.get_type_id());

		QList<const Core::TypeData*> type_parent_list;
		{
			const Core::TypeData* parent_type = type_data;
			while (parent_type != nullptr)
			{
				type_parent_list.push_back(parent_type);
				parent_type = data_schema.find_type_data(Utilities::vadon_uuid_to_qt_uuid(parent_type->info.base_id));
			}
		}

		// NOTE: this will create the widgets from the derived type first, going from top-to-bottom toward the parent type
		for (const Core::TypeData* current_type : type_parent_list)
		{
			// TODO: implement custom widgets for certain object types

			// Add a label for the property section
			{
				QString current_type_name = current_type->find_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME);
				if (current_type_name.isEmpty())
				{
					current_type_name = QString("Object type %1").arg(Utilities::vadon_uuid_to_qt_uuid(current_type->info.id).toString());
				}

				QLabel* section_label = new QLabel(current_type_name, parent_widget);
				section_label->setAlignment(Qt::AlignmentFlag::AlignCenter);

				section_label->setFrameStyle(QFrame::Panel);

				widget_list.push_back(section_label);
			}

			for (const ::Vadon::Foundation::UUID& property_uuid : current_type->property_list)
			{
				const Core::PropertyData* property_data = current_type->find_property_data(Utilities::vadon_uuid_to_qt_uuid(property_uuid));

				if (property_data->flags & ::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN)
				{
					// Skip properties that are hidden
					continue;
				}

				PropertyWidgetInfo widget_info;
				widget_info.property_id = Utilities::vadon_uuid_to_qt_uuid(property_uuid);
				widget_info.type_list = property_data->type_list;

				QVariant init_value;
				if (data_object.has_property(widget_info.property_id) == true)
				{
					init_value = data_object.get_property(widget_info.property_id);
				}
				else
				{
					init_value = data_object.get_property_default_value(widget_info.property_id);
				}
				widget_info.init_value = init_value;

				PropertyWidget* property_widget = PropertyWidget::create_widget(widget_info, parent_widget, owner_resource);
				if (property_widget == nullptr)
				{
					continue;
				}

				const QUuid property_qt_uuid = Utilities::vadon_uuid_to_qt_uuid(property_uuid);

				QString property_name = property_data->find_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME);
				if (property_name.isEmpty())
				{
					property_name = QString("Property %1").arg(property_qt_uuid.toString());
				}

				PropertyListEntry* list_entry = new PropertyListEntry(parent_widget, property_widget, property_name);
				widget_list.push_back(list_entry);
			}
		}

		return widget_list;
	}

	void ObjectEditor::internal_property_edited(const QUuid& property_id)
	{
		PropertyWidget* property_widget = find_property_widget(property_id);
		Q_ASSERT_X(property_widget != nullptr, "VadonEditor::UI::ObjectEditor::internal_property_edited", "Cannot find property widget");

		m_object.set_property(property_id, property_widget->get_value());
		emit(object_property_edited(property_id));
	}

	void ObjectEditor::initialize(Model::Resource* owner_resource)
	{
		QWidgetList widget_list = ObjectEditor::generate_property_widgets(m_object, owner_resource, this);
		for (QWidget* current_widget : widget_list)
		{
			PropertyListEntry* property_list_entry = qobject_cast<PropertyListEntry*>(current_widget);
			if (property_list_entry != nullptr)
			{
				connect(property_list_entry->get_property_widget(), &PropertyWidget::value_changed, this, &ObjectEditor::internal_property_edited);
			}

			// Insert before the spacer at the end
			const int spacer_index = m_ui.scrollContentsLayout->indexOf(m_ui.propertySpacer);
			m_ui.scrollContentsLayout->insertWidget(spacer_index, current_widget);
		}
	}

	PropertyWidget* ObjectEditor::find_property_widget(const QUuid& property_id) const
	{
		for (int item_index = 0; item_index < m_ui.scrollContentsLayout->count(); ++item_index)
		{
			QWidget* current_widget = m_ui.scrollContentsLayout->itemAt(item_index)->widget();
			PropertyListEntry* list_entry = qobject_cast<PropertyListEntry*>(current_widget);
			if (list_entry != nullptr)
			{
				PropertyWidget* property_widget = list_entry->get_property_widget();
				Q_ASSERT_X(property_widget != nullptr, "VadonEditor::UI::ObjectEditor::find_property_widget", "Cannot find property widget");
				if (property_widget->get_id() == property_id)
				{
					return property_widget;
				}
			}
		}

		return nullptr;
	}
}