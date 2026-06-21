#include <VadonEditor/UI/Model/Property/Resource.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/Scene.hpp>

#include <VadonEditor/UI/Project/Asset/AssetBrowser.hpp>

#include <VadonEditor/UI/Model/Resource/ResourceDialog.hpp>
#include <VadonEditor/UI/Model/Resource/ResourceEditor.hpp>

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
			QWidget* resource_widget = m_ui.resourceGroupBox->layout()->itemAt(0)->widget();
			ResourceEditor* resource_editor = qobject_cast<ResourceEditor*>(resource_widget);
			if (resource_editor != nullptr)
			{
				resource_editor->set_read_only(read_only);
			}
			else
			{
				// TODO: error!
			}
		}
	}

	void PropertyResource::resource_property_value_changed(const QUuid& id)
	{
		// TODO
		Q_UNUSED(id);
	}

	void PropertyResource::new_triggered()
	{
		NewResourceDialog* new_resource_dialog = new NewResourceDialog(m_owner_resource->get_application(), m_base_type, this);
		connect(new_resource_dialog, &NewResourceDialog::resource_type_selected, this, &PropertyResource::new_resource_type_selected);

		new_resource_dialog->open();
	}

	void PropertyResource::load_triggered()
	{
		// FIXME: instead of this, create dedicated dialog where we can search among just the Resource assets
		// filtered to the ones that are compatible in type
		OpenAssetDialog* open_asset_dialog = new OpenAssetDialog(m_owner_resource->get_application(), this);
		connect(open_asset_dialog, &OpenAssetDialog::asset_opened, this, &PropertyResource::resource_asset_opened);

		open_asset_dialog->open();
	}

	void PropertyResource::clear_triggered()
	{
		internal_set_value(QUuid());
		clear_resource_widgets();
	}

	void PropertyResource::new_resource_type_selected(const QUuid& resource_type)
	{
		if (is_resource_type_embeddable(m_owner_resource->get_application(), resource_type) == false)
		{
			QMessageBox::critical(this, "Resource Property Error", "Cannot embed this resource type!");
			return;
		}

		qDebug() << "Created new embedded resource, with type" << resource_type.toString();
	}

	void PropertyResource::resource_asset_opened(const QString& resource_path)
	{
		Core::Application& application = m_owner_resource->get_application();

		Model::ResourceSystem& resource_system = application.get_model_system().get_resource_system();
		const Core::AssetType required_asset_type = resource_system.get_asset_type_for_resource_type(m_base_type);

		Core::AssetManager& asset_manager = application.get_asset_manager();		

		const QModelIndex asset_index = asset_manager.find_asset_index_by_path(resource_path);
		if (asset_index.isValid() == false)
		{
			QMessageBox::critical(this, "Asset Library Error", QString("Cannot find asset at \"%1\"").arg(resource_path));
			return;
		}

		const Core::AssetInfo asset_info = asset_manager.get_asset_info(asset_index);
		if (asset_info.type != required_asset_type)
		{
			QMessageBox::critical(this, "Asset Library Error", "Asset type does not match resource type!");
			return;
		}

		const Model::ResourceInfo resource_info = resource_system.resource_info_by_asset_id(asset_info.id);
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

		qDebug() << "Opened resource" << resource_info.id << "at" << resource_path;
	}

	QUuid PropertyResource::get_resource_id() const
	{
		return get_value().toUuid();
	}

	Model::Resource* PropertyResource::find_resource() const
	{
		const QUuid resource_id = get_resource_id();
		if (resource_id.isNull() == true)
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

	void PropertyResource::generate_resource_widgets()
	{
		Model::Resource* resource = find_resource();
		if (resource != nullptr)
		{
			QVBoxLayout* vbox_layout = new QVBoxLayout();

			ResourceEditor* resource_editor = new ResourceEditor(resource, this);
			if (resource_editor->initialize() == false)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::PropertyResource::find_resource", "Cannot find resource!");
				clear_resource_widgets();
			}

			vbox_layout->addWidget(resource_editor);
			m_ui.resourceGroupBox->setLayout(vbox_layout);

			set_read_only(m_read_only);
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
			delete layout;
		}

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