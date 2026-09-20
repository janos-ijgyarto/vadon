#include <VadonEditor/UI/Model/Resource/ResourceDialog.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/UI/MainWindow.hpp>
#include <VadonEditor/UI/UISystem.hpp>

#include <VadonEditor/UI/Project/Asset/AssetDialog.hpp>

#include <Vadon/Foundation/Model/Resource/File.hpp>
#include <Vadon/Foundation/Model/Resource/Resource.hpp>
#include <Vadon/Foundation/Model/Scene/Scene.hpp>

#include <QMessageBox>

namespace
{
	bool is_resource_dialog_excluded_type(const QUuid& type_uuid)
	{
		// Scenes should only be created via the dedicated controls
		if (type_uuid == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneSchema::c_type_uuid))
		{
			return true;
		}

		// Files should only be imported via the dedicated controls
		if (type_uuid == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::FileResourceSchema::c_type_uuid))
		{
			return true;
		}

		return false;
	}
}

namespace VadonEditor::UI
{
	SelectResourceTypeDialog::SelectResourceTypeDialog(Core::Application& application, const QUuid& base_type, QWidget* parent)
		: NewObjectDialog(application, base_type, parent)
	{
		Q_ASSERT_X(Model::Resource::is_resource_base_of_type(application, base_type), "VadonEditor::UI::SelectResourceTypeDialog::SelectResourceTypeDialog", "Base type must be subclass of Resource!");
		setWindowTitle("Select Resource Type");
	}

	NewResourceDialog::NewResourceDialog(Core::Application& application, const QUuid& base_type, const QModelIndex& root_asset, QWidget* parent)
		: QDialog(parent)
		, m_application(application)
		, m_root_asset(root_asset)
	{
		m_ui.setupUi(this);
		setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);

		QUuid validated_base_type = base_type;
		if (Model::Resource::is_resource_base_of_type(application, validated_base_type) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::NewResourceDialog::NewResourceDialog", "Base type must be subclass of Resource!");
			const QUuid resource_base_type = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ResourceSchema::c_type_uuid);
			validated_base_type = resource_base_type;
		}

		m_base_type = validated_base_type;

		update_controls();
	}

	void NewResourceDialog::accept()
	{
		Q_ASSERT_X(Utilities::is_uuid_valid(m_new_resource_type) == true, "VadonEditor::UI::NewResourceDialog::accept", "Resource type not set");

		Q_ASSERT_X(m_new_resource_path.isEmpty() == false, "VadonEditor::UI::NewResourceDialog::accept", "Invalid path");

		// First verify that the asset doesn't already exist
		Core::AssetManager& asset_manager = m_application.get_asset_manager();
		if (asset_manager.find_asset_index_by_path(Core::AssetInfo::get_file_path(m_new_resource_path, Core::AssetType::RESOURCE)).isValid() == true)
		{
			QMessageBox::critical(this, "Asset Manager Error", "Asset file already exists!");
			return;
		}

		// Next try to create the resource
		Model::ResourceSystem& resource_system = m_application.get_model_system().get_resource_system();
		Model::Resource* new_resource = resource_system.create_resource(m_new_resource_type);

		if (new_resource == nullptr)
		{
			QMessageBox::critical(this, "Resource System Error", "Failed to create resource!");
			return;
		}

		// Create asset
		const int asset_id = resource_system.create_resource_asset(new_resource->get_info().id, m_new_resource_path);
		if (asset_id == Core::AssetInfo::c_invalid_file_id)
		{
			// TODO: delete resource?
			QMessageBox::critical(this, "Resource System Error", "Failed to create resource asset!");
			return;
		}

		// TODO: also print type!
		const QModelIndex asset_index = asset_manager.find_asset_index(asset_id);
		qDebug() << "Resource created at" << asset_manager.get_asset_info(asset_index).path;

		QDialog::accept();
	}

	void NewResourceDialog::select_type_clicked()
	{
		SelectResourceTypeDialog* select_type_dialog = new SelectResourceTypeDialog(m_application, m_base_type, this);
		connect(select_type_dialog, &NewObjectDialog::object_type_selected, this, &NewResourceDialog::resource_type_selected);

		select_type_dialog->open();
	}

	void NewResourceDialog::resource_type_selected(const QUuid& type_uuid)
	{
		if (is_resource_dialog_excluded_type(type_uuid) == true)
		{
			QMessageBox::warning(this, "Resource System Warning", "This resource type can only be created via the dedicated workflows!");
			return;
		}

		m_new_resource_type = type_uuid;

		const Core::DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		const Core::TypeData* type_data = data_schema.find_type_data(m_base_type);
		QString resource_type_name = type_data->find_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME);
		if (resource_type_name.isEmpty())
		{
			resource_type_name = QString("Resource type %1").arg(m_base_type.toString());
		}

		m_ui.typeLineEdit->setText(resource_type_name);

		// Clear path, user should select it again
		m_new_resource_path.clear();

		update_controls();
	}

	void NewResourceDialog::browse_file_path_clicked()
	{
		SaveAssetDialog* save_dialog = new SaveAssetDialog(m_application, this, m_root_asset);
		connect(save_dialog, &SaveAssetDialog::asset_saved, this, &NewResourceDialog::file_path_selected);

		save_dialog->setWindowTitle("Select Resource File Path");

		save_dialog->open();
	}

	void NewResourceDialog::file_path_selected(const QString& asset_path)
	{
		m_new_resource_path = asset_path;
		update_controls();
	}

	void NewResourceDialog::update_controls()
	{
		const bool is_type_valid = Utilities::is_uuid_valid(m_new_resource_type);
		QPushButton* ok_button = m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok);

		if (m_new_resource_path.isEmpty() == false)
		{
			m_ui.filePathLineEdit->setText(Core::AssetInfo::get_file_path(m_new_resource_path, Core::AssetType::RESOURCE));
		}
		else
		{
			m_ui.filePathLineEdit->clear();
		}

		if (is_type_valid == false)
		{
			m_ui.filePathBrowseButton->setEnabled(false);
			ok_button->setEnabled(false);
			return;
		}

		m_ui.filePathBrowseButton->setEnabled(true);
		if (m_new_resource_path.isEmpty() == true)
		{
			ok_button->setEnabled(false);
			return;
		}

		ok_button->setEnabled(true);
	}

	ResourceAssetFilterModel::ResourceAssetFilterModel(Core::Application& application, const QUuid& resource_type, QObject* parent)
		: QSortFilterProxyModel(parent)
		, m_application(application)
		, m_resource_type(resource_type)
	{
	}

	bool ResourceAssetFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
	{
		QModelIndex source_index = sourceModel()->index(source_row, 0, source_parent);

		// First check if any children passed filtering
		if (sourceModel()->hasChildren(source_index))
		{
			for (int child_index = 0; child_index < sourceModel()->rowCount(source_index); ++child_index)
			{
				if (filterAcceptsRow(child_index, source_index) == true)
				{
					// If child is accepted, the parent is accepted as well
					return true;
				}
			}
		}

		// Next check if we pass base filtering
		if (QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent) == false)
		{
			return false;
		}

		// Get asset info
		const Core::AssetInfo& asset_info = m_application.get_asset_manager().get_asset_info(source_index);
		if (asset_info.type == Core::AssetType::FOLDER)
		{
			return false;
		}

		// Check if it's a compatible resource type
		const Model::ResourceInfo resource_info = m_application.get_model_system().get_resource_system().resource_info_by_asset_id(asset_info.id);

		const Core::DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		return data_schema.is_base_of(m_resource_type, resource_info.type) == true;
	}

	SelectResourceDialog::SelectResourceDialog(Core::Application& application, const QUuid& resource_type, QWidget* dialog_parent)
		: QDialog(dialog_parent)
		, m_application(application)
		, m_filter_model(application, resource_type)
	{
		setAttribute(Qt::WA_DeleteOnClose, true);
		m_ui.setupUi(this);

		m_filter_model.setSourceModel(&m_application.get_asset_manager().get_model());

		m_ui.assetTree->setModel(&m_filter_model);
		m_ui.assetTree->expandAll();

		update_buttons();
	}

	void SelectResourceDialog::filter_text_changed(const QString& text)
	{
		// Reset selection
		m_ui.assetTree->selectionModel()->clearSelection();

		// Update filter
		m_filter_model.setFilterFixedString(text);

		// Re-expand tree
		m_ui.assetTree->expandAll();
	}

	void SelectResourceDialog::tree_item_clicked(const QModelIndex& index)
	{
		Q_UNUSED(index);
		update_buttons();
	}

	void SelectResourceDialog::tree_item_double_clicked(const QModelIndex& index)
	{
		resource_selected(index);
	}

	void SelectResourceDialog::selection_accepted()
	{
		QItemSelectionModel* tree_selection_model = m_ui.assetTree->selectionModel();
		if (tree_selection_model->hasSelection() == false)
		{
			qCritical() << "No resource was selected!";
			return;
		}

		const QModelIndex selected_item = tree_selection_model->selectedIndexes().front();
		resource_selected(selected_item);
	}

	void SelectResourceDialog::resource_selected(const QModelIndex& index)
	{
		const Model::ResourceInfo resource_info = get_resource_info(index);
		if (is_compatible_item(resource_info))
		{
			emit(resource_asset_selected(resource_info.id));
			accept();
		}
	}

	void SelectResourceDialog::update_buttons()
	{
		QPushButton* ok_button = m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok);

		QItemSelectionModel* tree_selection_model = m_ui.assetTree->selectionModel();
		if (tree_selection_model->hasSelection())
		{
			const QModelIndex selected_item = tree_selection_model->selectedIndexes().front();
			const Model::ResourceInfo resource_info = get_resource_info(selected_item);

			if(is_compatible_item(resource_info) == true)
			{
				ok_button->setEnabled(true);
				return;
			}
		}

		ok_button->setEnabled(false);
	}

	bool SelectResourceDialog::is_compatible_item(const Model::ResourceInfo& resource_info) const
	{
		if (resource_info.is_valid() == true)
		{
			const Core::DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
			return data_schema.is_base_of(m_filter_model.get_resource_type(), resource_info.type);
		}

		return false;
	}

	Model::ResourceInfo SelectResourceDialog::get_resource_info(const QModelIndex& index) const
	{
		const QModelIndex source_index = m_filter_model.mapToSource(index);
		const Core::AssetInfo& asset_info = m_application.get_asset_manager().get_asset_info(source_index);
		if (asset_info.type == Core::AssetType::FOLDER)
		{
			return Model::ResourceInfo();
		}

		return m_application.get_model_system().get_resource_system().resource_info_by_asset_id(asset_info.id);
	}
}