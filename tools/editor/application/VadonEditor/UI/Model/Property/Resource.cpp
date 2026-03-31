#include <VadonEditor/UI/Model/Property/Resource.hpp>

#include <VadonEditor/Core/Application.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/UI/Project/Asset/AssetBrowser.hpp>

#include <VadonEditor/UI/Model/Resource/ResourceDialog.hpp>
#include <VadonEditor/UI/Model/Resource/ResourceEditor.hpp>

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

		m_ui.resourceToolButton->addAction(m_ui.actionNew);
		m_ui.resourceToolButton->addAction(m_ui.actionLoad);
		m_ui.resourceToolButton->addAction(m_ui.actionClear);

		connect(m_ui.actionNew, &QAction::triggered, this, &PropertyResource::new_triggered);
		connect(m_ui.actionLoad, &QAction::triggered, this, &PropertyResource::load_triggered);
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
	}

	void PropertyResource::clear_triggered()
	{
		internal_set_value(QUuid());
		clear_resource_widgets();
	}

	void PropertyResource::new_resource_type_selected(const QUuid& resource_type)
	{
		// TODO!!!
		//m_owner_resource->create_embedded_resource(resource_type);

		qDebug() << "Created new embedded resource, with type" << resource_type.toString();
	}

	void PropertyResource::resource_asset_opened(const QString& resource_path)
	{
		qDebug() << "Opened resource" << resource_path;
	}

	QUuid PropertyResource::get_resource_id() const
	{
		return get_value().toUuid();
	}

	Model::Resource* PropertyResource::find_resource() const
	{
		const QUuid resource_id = get_resource_id();
		if (resource_id.isNull() == false)
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
	}
}