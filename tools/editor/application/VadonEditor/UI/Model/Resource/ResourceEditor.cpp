#include <VadonEditor/UI/Model/Resource/ResourceEditor.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/UI/Model/Property/Property.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <QCloseEvent>
#include <QMessageBox>
#include <QToolBar>

namespace VadonEditor::UI
{
	ResourceEditor::ResourceEditor(Model::Resource* resource, QWidget* parent, Qt::WindowType type)
		: QWidget(parent, type)
		, m_resource(resource)
	{
		m_ui.setupUi(this);
		setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);
	}

	bool ResourceEditor::initialize()
	{
		if (m_resource == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::ResourceEditor::initialize", "Resource must not be null!");
			return false;
		}

		update_title();

		if (windowType() == Qt::WindowType::Window)
		{
			// No parent, can add toolbar
			QToolBar* toolbar = new QToolBar();
			toolbar->addAction(m_ui.actionSave);

			m_ui.mainLayout->setMenuBar(toolbar);
		}

		const VadonEditor::Model::ResourceInfo resource_info = m_resource->get_info();
		Core::Application& application = m_resource->get_application();

		const Core::DataSchema& data_schema = application.get_project_manager().get_project_data_schema();
		const Core::TypeData* type_data = data_schema.find_type_data(resource_info.type);

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
			// Add a label for the property section
			{
				QString current_type_name = current_type->find_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME);
				if (current_type_name.isEmpty())
				{
					current_type_name = QString("Resource type %1").arg(Utilities::vadon_uuid_to_qt_uuid(current_type->info.id).toString());
				}
				const int spacer_index = m_ui.propertyListVBox->indexOf(m_ui.propertyListSpacer);

				QLabel* section_label = new QLabel(current_type_name, this);
				section_label->setAlignment(Qt::AlignmentFlag::AlignCenter);

				m_ui.propertyListVBox->insertWidget(spacer_index, section_label);
			}

			for (const ::Vadon::Foundation::UUID& property_uuid : current_type->property_list)
			{
				const Core::PropertyData* property_data = current_type->find_property_data(Utilities::vadon_uuid_to_qt_uuid(property_uuid));

				PropertyWidgetInfo widget_info;
				widget_info.property_id = Utilities::vadon_uuid_to_qt_uuid(property_uuid);
				widget_info.category = property_data->get_category();
				widget_info.data_type = property_data->get_data_type();
				widget_info.init_value = m_resource->get_property(widget_info.property_id);

				PropertyWidget* property_widget = PropertyWidget::create_widget(widget_info, this, m_resource);
				if (property_widget == nullptr)
				{
					continue;
				}

				connect(property_widget, &PropertyWidget::value_changed, this, &ResourceEditor::internal_property_edited);

				const QUuid property_qt_uuid = Utilities::vadon_uuid_to_qt_uuid(property_uuid);

				QString property_name = property_data->find_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME);
				if (property_name.isEmpty())
				{
					property_name = QString("Property %1").arg(property_qt_uuid.toString());
				}

				PropertyListEntry* list_entry = new PropertyListEntry(this, property_widget, property_name);

				const int spacer_index = m_ui.propertyListVBox->indexOf(m_ui.propertyListSpacer);
				m_ui.propertyListVBox->insertWidget(spacer_index, list_entry);
			}
		}

		// TODO: add list of embedded resources, and the option to delete
		// TODO2: make sure we refcount so they dont get deleted while they are referenced!

		return true;
	}

	QString ResourceEditor::get_label() const
	{
		Core::Application& application = m_resource->get_application();
		const VadonEditor::Model::ResourceInfo resource_info = m_resource->get_info();

		// Set label based on asset path
		QString resource_path;
		{
			// If nested resource, use the "path" from parents
			// FIXME: instead of the verbose UUIDs, use the property IDs/names
			const Core::AssetManager& asset_manager = application.get_asset_manager();

			const Model::Resource* current_resource = m_resource;
			QString asset_path;
			while (current_resource->get_owner())
			{
				// FIXME: replace this with the property name!
				QString resource_id_string = current_resource->get_info().id.toString();
				asset_path = asset_path.isEmpty() ? resource_id_string : QString("%1/%2").arg(resource_id_string).arg(asset_path);

				current_resource = current_resource->get_owner();
			}

			Model::ResourceSystem& resource_system = application.get_model_system().get_resource_system();
			const int resource_asset_id = resource_system.find_resource_asset_id(resource_info.id);

			if (resource_asset_id == Core::AssetInfo::c_invalid_file_id)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::ResourceEditor::get_label", "Cannot find resource asset");
				return resource_path;
			}

			const QModelIndex asset_index = asset_manager.find_asset_index(resource_asset_id);
			if (asset_index.isValid() == false)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::ResourceEditor::get_label", "Cannot find asset");
				return resource_path;
			}

			const Core::AssetInfo resource_asset_info = asset_manager.get_asset_info(asset_index);

			resource_path = asset_path.isEmpty() == false ? QString("%1/%2").arg(resource_asset_info.path).arg(asset_path) : resource_asset_info.path;
		}

		const Core::DataSchema& data_schema = application.get_project_manager().get_project_data_schema();
		const Core::TypeData* type_data = data_schema.find_type_data(resource_info.type);

		QString type_name = type_data->find_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME);

		if (type_name.isEmpty())
		{
			type_name = QString("Resource type %1").arg(resource_info.type.toString());
		}

		return QString("%1 (%2)").arg(resource_path).arg(type_name);
	}

	void ResourceEditor::set_read_only(bool read_only)
	{
		for (int item_index = 0; item_index < m_ui.propertyListVBox->count(); ++item_index)
		{
			QWidget* current_widget = m_ui.propertyListVBox->itemAt(item_index)->widget();
			PropertyListEntry* list_entry = qobject_cast<PropertyListEntry*>(current_widget);
			if (list_entry != nullptr)
			{
				PropertyWidget* property_widget = list_entry->get_property_widget();
				Q_ASSERT_X(property_widget != nullptr, "VadonEditor::UI::ResourceEditor::set_read_only", "Cannot find property widget");
				property_widget->set_read_only(read_only);
			}
		}
	}

	bool ResourceEditor::request_close()
	{
		if (m_resource->is_modified() == true)
		{
			QMessageBox message_box(this);
			message_box.setWindowTitle("Unsaved changes in Resource");
			message_box.setText(QString("Save changes to \"%1?\"").arg(get_label()));
			message_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
			message_box.setDefaultButton(QMessageBox::Yes);
			message_box.setIcon(QMessageBox::Icon::Question);
			
			const int user_response = message_box.exec();
			switch(user_response)
			{
			case QMessageBox::StandardButton::Yes:
				save_triggered();
				break;
			case QMessageBox::StandardButton::No:
				reload_triggered();
				break;
			case QMessageBox::StandardButton::Cancel:
				return false;
			}
		}

		return true;
	}

	void ResourceEditor::closeEvent(QCloseEvent* event)
	{
		if (windowType() == Qt::WindowType::Window)
		{
			if (request_close() == false)
			{
				event->ignore();
				return;
			}
		}

		QWidget::closeEvent(event);
	}

	void ResourceEditor::internal_property_edited(const QUuid& property_id)
	{
		PropertyWidget* property_widget = find_property_widget(property_id);
		Q_ASSERT_X(property_widget != nullptr, "VadonEditor::UI::ResourceEditor::internal_property_edited", "Cannot find property widget");

		m_resource->set_property(property_id, property_widget->get_value());

		update_title();
	}

	void ResourceEditor::save_triggered()
	{
		if (m_resource->is_modified() == false)
		{
			// Nothing to save
			return;
		}

		Core::Application& application = m_resource->get_application();
		Model::ResourceSystem& resource_system = application.get_model_system().get_resource_system();
		if (resource_system.save_resource(m_resource) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::ResourceEditor::save_triggered", "Failed to save resource");
			return;
		}

		update_title();
	}

	void ResourceEditor::reload_triggered()
	{
		Core::Application& application = m_resource->get_application();
		Model::ResourceSystem& resource_system = application.get_model_system().get_resource_system();
		if (resource_system.reload_resource(m_resource) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::ResourceEditor::reload_triggered", "Failed to reload resource");
			return;
		}

		update_title();
	}

	void ResourceEditor::update_title()
	{
		if (windowType() == Qt::WindowType::Window)
		{
			// Opened as separate window, so we should set a title
			QString title_text = QString("Resource Editor - %1").arg(get_label());
			if (m_resource->is_modified() == true)
			{
				title_text += " (*)";
			}
			setWindowTitle(title_text);
		}
	}

	PropertyWidget* ResourceEditor::find_property_widget(const QUuid& property_id) const
	{
		for (int item_index = 0; item_index < m_ui.propertyListVBox->count(); ++item_index)
		{
			QWidget* current_widget = m_ui.propertyListVBox->itemAt(item_index)->widget();
			PropertyListEntry* list_entry = qobject_cast<PropertyListEntry*>(current_widget);
			if (list_entry != nullptr)
			{
				PropertyWidget* property_widget = list_entry->get_property_widget();
				Q_ASSERT_X(property_widget != nullptr, "VadonEditor::UI::ResourceEditor::find_property_widget", "Cannot find property widget");
				if (property_widget->get_id() == property_id)
				{
					return property_widget;
				}
			}
		}

		return nullptr;
	}
}