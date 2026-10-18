#include <VadonEditor/UI/Model/Scene/ComponentWidget.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/Animation/Animation.hpp>

#include <VadonEditor/Model/Scene/Component.hpp>
#include <VadonEditor/Model/Scene/Entity.hpp>
#include <VadonEditor/Model/Scene/Scene.hpp>

#include <VadonEditor/UI/Model/Animation/AnimationComponentWidget.hpp>
#include <VadonEditor/UI/Model/Property/Property.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <QMessageBox>

namespace VadonEditor::UI
{
	ComponentWidget::ComponentWidget(Model::Scene* owner_scene, Model::Entity* entity, Model::Component* component, QWidget* parent)
		: QWidget(parent)
		, m_owner_scene(owner_scene)
		, m_entity(entity)
		, m_component(component)
	{
		m_ui.setupUi(this);
	}

	bool ComponentWidget::initialize()
	{
		const Core::DataSchema& data_schema = m_component->get_application().get_project_manager().get_project_data_schema();
		const Core::TypeData* type_data = data_schema.find_type_data(m_component->get_type_id());

		if (m_component->is_mandatory() == true)
		{
			m_ui.removeButton->setEnabled(false);
			m_ui.removeButton->setVisible(false);
		}

		if (type_data == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::ComponentWidget::initialize", "Cannot find type data!");
			return false;
		}

		// FIXME: find a more elegant way to create a component widget for animations?
		if (m_component->get_type_id() == Model::Animation::get_animation_component_type_uuid())
		{
			QPushButton* edit_animation_button = new QPushButton("Edit Animation", this);
			connect(edit_animation_button, &QPushButton::clicked, this, &ComponentWidget::edit_animation_clicked);

			m_ui.propertiesVBox->addWidget(edit_animation_button);
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

			if (m_component->is_mandatory() == true)
			{
				component_type_name += " (mandatory)";
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

				QLabel* section_label = new QLabel(current_type_name, this);
				section_label->setAlignment(Qt::AlignmentFlag::AlignCenter);

				m_ui.propertiesVBox->addWidget(section_label);
			}

			for (const ::Vadon::Foundation::UUID& property_uuid : current_type->property_list)
			{
				const QUuid property_qt_uuid = Utilities::vadon_uuid_to_qt_uuid(property_uuid);
				const Core::PropertyData* property_data = current_type->find_property_data(property_qt_uuid);

				if (property_data->flags & ::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN)
				{
					// Skip properties that are hidden
					continue;
				}

				PropertyWidgetInfo widget_info;
				widget_info.property_id = property_qt_uuid;
				widget_info.type_list = property_data->type_list;

				const Core::DataObject& component_data = m_component->get_data_object();
				if (component_data.has_property(widget_info.property_id) == true)
				{
					widget_info.init_value = component_data.get_property(widget_info.property_id);
				}
				else
				{
					widget_info.init_value = component_data.get_property_default_value(widget_info.property_id);
				}

				PropertyWidget* property_widget = PropertyWidget::create_widget(m_component->get_application(), widget_info, this, m_owner_scene->get_resource());
				if (property_widget == nullptr)
				{
					continue;
				}

				connect(property_widget, &PropertyWidget::value_changed, this, &ComponentWidget::internal_property_edited);

				QString property_name = property_data->find_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME);
				if (property_name.isEmpty())
				{
					property_name = QString("Property %1").arg(property_qt_uuid.toString());
				}

				PropertyListEntry* list_entry = new PropertyListEntry(this, property_widget, property_name);

				m_ui.propertiesVBox->addWidget(list_entry);
			}
		}

		// TODO: add list of embedded resources, and the option to delete
		// TODO2: make sure we refcount so they dont get deleted while they are referenced!

		return true;
	}

	void ComponentWidget::internal_property_edited(const QUuid& property_id)
	{
		PropertyWidget* property_widget = find_property_widget(property_id);

		m_component->set_property(property_id, property_widget->get_value());
	}

	void ComponentWidget::remove_clicked()
	{
		const QMessageBox::StandardButton user_response = QMessageBox::question(this, "Remove Component", QString("Are you sure you want to remove %1").arg(m_ui.componentNameLabel->text()));
		if (user_response == QMessageBox::StandardButton::Yes)
		{
			emit(remove_requested(m_component->get_type_id()));
		}
	}

	void ComponentWidget::edit_animation_clicked()
	{
		AnimationComponentWidget* anim_component_widget = new AnimationComponentWidget(m_owner_scene, m_entity, m_component);
		anim_component_widget->show();
	}

	PropertyWidget* ComponentWidget::find_property_widget(const QUuid& property_id) const
	{
		for (int item_index = 0; item_index < m_ui.propertiesVBox->count(); ++item_index)
		{
			QWidget* current_widget = m_ui.propertiesVBox->itemAt(item_index)->widget();
			PropertyListEntry* list_entry = qobject_cast<PropertyListEntry*>(current_widget);
			if (list_entry != nullptr)
			{
				PropertyWidget* property_widget = list_entry->get_property_widget();
				Q_ASSERT_X(property_widget != nullptr, "VadonEditor::UI::ComponentWidget::find_property_widget", "Cannot find property widget");
				if (property_widget->get_id() == property_id)
				{
					return property_widget;
				}
			}
		}

		return nullptr;
	}
}