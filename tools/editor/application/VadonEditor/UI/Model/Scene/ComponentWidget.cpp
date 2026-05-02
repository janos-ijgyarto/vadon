#include <VadonEditor/UI/Model/Scene/ComponentWidget.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/Scene/Component.hpp>
#include <VadonEditor/Model/Scene/Entity.hpp>
#include <VadonEditor/Model/Scene/Scene.hpp>

#include <VadonEditor/UI/Model/Property/Property.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <QMessageBox>

namespace VadonEditor::UI
{
	ComponentWidget::ComponentWidget(Model::Entity* entity, Model::Component* component, QWidget* parent)
		: QWidget(parent)
		, m_entity(entity)
		, m_component(component)
	{
		m_ui.setupUi(this);
	}

	bool ComponentWidget::initialize()
	{
		Core::Application& application = m_entity->get_owner_scene().get_application();
		const Core::DataSchema& data_schema = application.get_project_manager().get_project_data_schema();
		const Core::TypeData* type_data = data_schema.find_type_data(m_component->type_id);

		if (type_data == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::ComponentWidget::initialize", "Cannot find type data!");
			return false;
		}

		QList<const Core::TypeData*> type_parent_list;
		{
			const Core::TypeData* parent_type = type_data;
			while (parent_type != nullptr)
			{
				type_parent_list.push_back(parent_type);
				parent_type = data_schema.find_type_data(Utilities::vadon_uuid_to_qt_uuid(parent_type->info.base_id));
			}
		}

		{
			// Add a label for the property section
			QString component_type_name = type_data->find_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME);
			if (component_type_name.isEmpty())
			{
				component_type_name = QString("Component type %1").arg(Utilities::vadon_uuid_to_qt_uuid(type_data->info.id).toString());
			}

			m_ui.componentNameLabel->setText(component_type_name);
		}

		// NOTE: this will create the widgets from the derived type first, going from top-to-bottom toward the parent type
		for (const Core::TypeData* current_type : type_parent_list)
		{
			if(type_parent_list.size() > 1)
			{
				// Add a label for the property section
				QString current_type_name = current_type->find_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME);
				if (current_type_name.isEmpty())
				{
					current_type_name = QString("Component type %1").arg(Utilities::vadon_uuid_to_qt_uuid(current_type->info.id).toString());
				}
				const int spacer_index = m_ui.propertiesVBox->indexOf(m_ui.propertiesSpacer);

				QLabel* section_label = new QLabel(current_type_name, this);
				section_label->setAlignment(Qt::AlignmentFlag::AlignCenter);

				m_ui.propertiesVBox->insertWidget(spacer_index, section_label);
			}

			for (const ::Vadon::Foundation::UUID& property_uuid : current_type->property_list)
			{
				const Core::PropertyData* property_data = current_type->find_property_data(Utilities::vadon_uuid_to_qt_uuid(property_uuid));

				PropertyWidgetInfo widget_info;
				widget_info.property_id = Utilities::vadon_uuid_to_qt_uuid(property_uuid);
				widget_info.category = property_data->get_category();
				widget_info.data_type = property_data->get_data_type();
				widget_info.init_value = m_component->get_property(widget_info.property_id);

				PropertyWidget* property_widget = PropertyWidget::create_widget(widget_info, this, m_entity->get_owner_scene().get_resource());
				if (property_widget == nullptr)
				{
					continue;
				}

				connect(property_widget, &PropertyWidget::value_changed, this, &ComponentWidget::internal_property_edited);

				const QUuid property_qt_uuid = Utilities::vadon_uuid_to_qt_uuid(property_uuid);

				QString property_name = property_data->find_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME);
				if (property_name.isEmpty())
				{
					property_name = QString("Property %1").arg(property_qt_uuid.toString());
				}

				PropertyListEntry* list_entry = new PropertyListEntry(this, property_widget, property_name);

				const int spacer_index = m_ui.propertiesVBox->indexOf(m_ui.propertiesSpacer);
				m_ui.propertiesVBox->insertWidget(spacer_index, list_entry);
			}
		}

		// TODO: add list of embedded resources, and the option to delete
		// TODO2: make sure we refcount so they dont get deleted while they are referenced!

		return true;
	}

	void ComponentWidget::internal_property_edited(const QUuid& property_id)
	{
		emit(property_edited(m_component->type_id, property_id));
	}

	void ComponentWidget::remove_clicked()
	{
		const QMessageBox::StandardButton user_response = QMessageBox::question(this, "Remove Component", QString("Are you sure you want to remove %1").arg(m_ui.componentNameLabel->text()));
		if (user_response == QMessageBox::StandardButton::Yes)
		{
			emit(remove_requested(m_component->type_id));
		}
	}
}