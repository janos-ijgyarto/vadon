#include <VadonEditor/UI/Model/Property/Resource.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Animation/Animation.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/Scene.hpp>

#include <VadonEditor/UI/Model/Resource/ResourceDialog.hpp>
#include <VadonEditor/UI/Model/Resource/ResourceEditor.hpp>

#include <VadonEditor/UI/Project/Asset/AssetDialog.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <QMessageBox>

namespace
{
	bool is_resource_type_embeddable(VadonEditor::Core::Application& application, const QUuid& resource_type)
	{
		// FIXME: do this via type metadata instead of hardcoded checks?
		if (VadonEditor::Model::Scene::is_scene_base_of_type(application, resource_type) == true)
		{
			return false;
		}

		if (VadonEditor::Model::Resource::is_imported_file_base_of_type(application, resource_type) == true)
		{
			return false;
		}

		if (VadonEditor::Model::Animation::is_animation_base_of_type(application, resource_type) == true)
		{
			return false;
		}

		return true;
	}
}

namespace VadonEditor::UI
{
	PropertyResource::PropertyResource(const QUuid& id, Model::Resource* resource, Model::Resource* owner_resource, const QUuid& base_type, QWidget* parent)
		: PropertyWidget(id, resource != nullptr ? resource->get_info().id : QUuid(), parent)
		, m_resource(resource)
		, m_owner_resource(owner_resource)
		, m_base_type(base_type)
		, m_read_only(false)
	{
		// This widget can only exist for a property inside an existing resource!
		Q_ASSERT_X(owner_resource != nullptr, "VadonEditor::UI::PropertyResource::PropertyResource", "Owner must not be null!");
		if (Model::Resource::is_resource_base_of_type(owner_resource->get_application(), base_type) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::PropertyResource::PropertyResource", "Base type must be subclass of Resource!");
			m_base_type = Model::Resource::get_base_resource_type();
		}

		m_ui.setupUi(this);

		if (is_resource_type_embeddable(owner_resource->get_application(), m_base_type) == true)
		{
			m_ui.resourceToolButton->addAction(m_ui.actionNew);
			connect(m_ui.actionNew, &QAction::triggered, this, &PropertyResource::new_triggered);
		}

		m_ui.resourceToolButton->addAction(m_ui.actionLoad);
		connect(m_ui.actionLoad, &QAction::triggered, this, &PropertyResource::load_triggered);

		m_ui.resourceToolButton->addAction(m_ui.actionClear);
		connect(m_ui.actionClear, &QAction::triggered, this, &PropertyResource::clear_triggered);

		generate_resource_widgets();
	}

	void PropertyResource::set_read_only(bool read_only)
	{
		m_read_only = read_only;

		m_ui.resourceToolButton->setVisible(read_only == false);
		m_ui.resourceToolButton->setEnabled(read_only == false);

		Model::Resource* resource = find_resource();
		if (resource != nullptr)
		{
			for(int item_index = 0; item_index < m_ui.resourceGroupBox->layout()->count(); ++item_index)
			{
				QWidget* resource_widget = m_ui.resourceGroupBox->layout()->itemAt(item_index)->widget();
				ResourceEditor* resource_editor = qobject_cast<ResourceEditor*>(resource_widget);
				if (resource_editor != nullptr)
				{
					resource_editor->set_read_only(read_only || (resource->is_embedded() == false));
					return;
				}
				else
				{
					// TODO: error!
				}
			}
		}
	}

	void PropertyResource::resource_property_value_changed(const QUuid& id)
	{
		Q_UNUSED(id);

		// Propagate the change notification
		emit(value_changed(get_id()));
	}

	void PropertyResource::new_triggered()
	{
		NewResourceDialog* new_resource_dialog = new NewResourceDialog(m_owner_resource->get_application(), m_base_type, this);
		connect(new_resource_dialog, &NewResourceDialog::resource_type_selected, this, &PropertyResource::new_resource_type_selected);

		new_resource_dialog->open();
	}

	void PropertyResource::load_triggered()
	{
		SelectResourceDialog* select_dialog = new SelectResourceDialog(m_owner_resource->get_application(), m_base_type, this);
		connect(select_dialog, &SelectResourceDialog::resource_asset_selected, this, &PropertyResource::resource_asset_opened);

		select_dialog->open();
	}

	void PropertyResource::clear_triggered()
	{
		const QUuid resource_id = get_resource_id();
		if (Utilities::is_uuid_valid(resource_id) == false)
		{
			return;
		}

		Model::Resource* resource = m_owner_resource->get_application().get_model_system().get_resource_system().find_resource(resource_id);
		Q_ASSERT_X(resource != nullptr, "VadonEditor::UI::PropertyResource::clear_triggered", "Cannot find resource");

		internal_set_value(QUuid());
		clear_resource_widgets();

		if (resource->is_embedded() == true)
		{
			// Delete embedded resource
			delete resource;
		}
	}

	void PropertyResource::new_resource_type_selected(const QUuid& resource_type)
	{
		if (is_resource_type_embeddable(m_owner_resource->get_application(), resource_type) == false)
		{
			QMessageBox::critical(this, "Resource Property Error", "Cannot embed this resource type!");
			return;
		}

		Model::Resource* embedded_resource = m_owner_resource->create_embedded_resource(resource_type);
		internal_set_resource(embedded_resource);
	}

	void PropertyResource::resource_asset_opened(const QUuid& resource_id)
	{
		Core::Application& application = m_owner_resource->get_application();
		Model::ResourceSystem& resource_system = application.get_model_system().get_resource_system();

		const int resource_asset_id = resource_system.find_resource_asset_id(resource_id);
		if (resource_asset_id == Core::AssetInfo::c_invalid_file_id)
		{
			QMessageBox::critical(this, "Resource System Error", "Cannot find asset for Resource!");
			return;
		}

		const Model::ResourceInfo resource_info = resource_system.resource_info_by_asset_id(resource_asset_id);
		if (resource_info.is_valid() == false)
		{
			QMessageBox::critical(this, "Resource System Error", "Cannot find resource for this asset!");
			return;
		}

		if (application.get_project_manager().get_project_data_schema().is_base_of(m_base_type, resource_info.type) == false)
		{
			QMessageBox::critical(this, "Resource System Error", "Resource type is not compatible with property!");
			return;
		}

		Model::Resource* selected_resource = m_owner_resource->get_application().get_model_system().get_resource_system().get_resource(resource_id);
		internal_set_resource(selected_resource);
	}

	QUuid PropertyResource::get_resource_id() const
	{
		return get_value().toUuid();
	}

	Model::Resource* PropertyResource::find_resource() const
	{
		const QUuid resource_id = get_resource_id();
		if (Utilities::is_uuid_valid(resource_id) == false)
		{
			return nullptr;
		}

		Core::Application& application = m_owner_resource->get_application();
		Model::Resource* resource = application.get_model_system().get_resource_system().get_resource(resource_id);
		if (resource == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::PropertyResource::find_resource", "Cannot find resource!");
			return nullptr;
		}

		return resource;
	}

	void PropertyResource::internal_set_resource(Model::Resource* resource)
	{
		// Clear previous contents
		clear_triggered();

		if (resource != nullptr)
		{
			internal_set_value(resource->get_info().id);
			generate_resource_widgets();
		}
		else
		{
			Q_ASSERT_X(false, "VadonEditor::UI::PropertyResource::internal_set_resource", "Invalid resource");
		}
	}

	void PropertyResource::generate_resource_widgets()
	{
		Model::Resource* resource = find_resource();
		if (resource != nullptr)
		{
			QVBoxLayout* vbox_layout = new QVBoxLayout();

			// FIXME: instead of creating it here, we should query it from the ResourceManager
			// This allows it to track each "View" onto the same resource and connect signals, ensuring they
			// can be updated if they are edited and viewed from multiple locations
			ResourceEditor* resource_editor = new ResourceEditor(resource, this);
			if (resource_editor->initialize() == false)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::PropertyResource::generate_resource_widgets", "Failed to initialize resource editor!");
				clear_resource_widgets();
			}

			connect(resource_editor, &ResourceEditor::resource_property_edited, this, &PropertyResource::resource_property_value_changed);

			const Model::ResourceInfo& resource_info = resource->get_info();

			Core::Application& application = m_owner_resource->get_application();
			const Core::DataSchema& data_schema = application.get_project_manager().get_project_data_schema();
			const Core::TypeData* type_data = data_schema.find_type_data(resource_info.type);

			QString current_type_name = type_data->find_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME);
			if (current_type_name.isEmpty())
			{
				current_type_name = QString("Resource type %1").arg(m_base_type.toString());
			}

			QString label_string = current_type_name;
			if (resource->is_embedded() == false)
			{
				const int resource_asset_id = application.get_model_system().get_resource_system().find_resource_asset_id(resource_info.id);
				Q_ASSERT_X(resource_asset_id != Core::AssetInfo::c_invalid_file_id, "VadonEditor::UI::PropertyResource::generate_resource_widgets", "Cannot find resource asset");

				Core::AssetManager& asset_manager = application.get_asset_manager();
				const QModelIndex asset_index = asset_manager.find_asset_index(resource_asset_id);
				const Core::AssetInfo asset_info = application.get_asset_manager().get_asset_info(asset_index);
				label_string += QString(" (%1) - %2").arg(Utilities::uuid_to_base64_string(resource_info.id)).arg(asset_info.path);
			}
			else
			{
				// TODO: create a "path" of resource properties for embedded resource
				label_string += QString(" (%1)").arg(Utilities::uuid_to_base64_string(resource_info.id));
			}

			QLabel* resource_label = new QLabel(label_string);
			resource_label->setWordWrap(true);

			vbox_layout->addWidget(resource_label);
			vbox_layout->addWidget(resource_editor);
			m_ui.resourceGroupBox->setLayout(vbox_layout);

			set_read_only(m_read_only);

			setMinimumSize(QSize(400, 300));
		}
		else
		{
			clear_resource_widgets();
		}
	}

	void PropertyResource::clear_resource_widgets()
	{
		QLayout* layout = m_ui.resourceGroupBox->layout();
		if (layout != nullptr)
		{
			QLayoutItem* current_item = nullptr;
			while (current_item = layout->takeAt(0))
			{
				if (current_item->widget())
				{
					delete current_item->widget();
				}
				delete current_item;
			}
			delete layout;
		}
		
		// Reset minimum size
		m_ui.resourceGroupBox->adjustSize();
		setMinimumSize(QSize(0, 0));
		adjustSize();

		const Core::DataSchema& data_schema = m_owner_resource->get_application().get_project_manager().get_project_data_schema();
		const Core::TypeData* type_data = data_schema.find_type_data(m_base_type);
		QString current_type_name = type_data->find_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME);
		if (current_type_name.isEmpty())
		{
			current_type_name = QString("Resource type %1").arg(m_base_type.toString());
		}

		m_ui.resourceNameLabel->setText(current_type_name);
	}
}