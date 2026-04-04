#include <VadonEditor/UI/Model/Resource/ResourceDialog.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>
#include <VadonEditor/Core/Data/Schema.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/UI/MainWindow.hpp>
#include <VadonEditor/UI/UISystem.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

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
	NewResourceDialog::NewResourceDialog(Core::Application& application, const QUuid& base_type, QWidget* parent)
		: QDialog(parent)
	{
		setAttribute(Qt::WA_DeleteOnClose, true);

		m_ui.setupUi(this);

		QUuid validated_base_type = base_type;
		if (Model::Resource::is_resource_base_of_type(application, validated_base_type) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::NewResourceDialog::NewResourceDialog", "Base type must be subclass of Resource!");
			const QUuid resource_base_type = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ResourceSchema::c_type_uuid);
			validated_base_type = resource_base_type;
		}

		const Core::DataSchema& data_schema = application.get_project_manager().get_project_data_schema();
		m_ui.typeTreeView->setModel(const_cast<QStandardItemModel*>(&data_schema.get_qt_model()));

		const QModelIndex root_type_index = data_schema.find_type_index(validated_base_type);
		if (root_type_index.isValid() == true)
		{
			const QModelIndex parent_index = root_type_index.parent();
			m_ui.typeTreeView->setRootIndex(parent_index);
		
			const int row_count = m_ui.typeTreeView->model()->rowCount(parent_index);
			for (int current_row = 0; current_row < row_count; ++current_row)
			{
				if (root_type_index.row() != current_row)
				{
					m_ui.typeTreeView->setRowHidden(current_row, parent_index, true);
				}
			}
		}

		connect(m_ui.typeTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &NewResourceDialog::selection_changed);

		update_controls();
	}

	void NewResourceDialog::type_double_clicked(const QModelIndex& index)
	{
		const QUuid selected_type = get_selected_type(index);
		finalize_selection(selected_type);
	}

	void NewResourceDialog::selection_changed(const QItemSelection& selected, const QItemSelection& deselected)
	{
		Q_UNUSED(selected);
		Q_UNUSED(deselected);
		update_controls();
	}

	void NewResourceDialog::selection_accepted()
	{
		const QUuid selected_type = get_selected_type(get_current_selection());
		finalize_selection(selected_type);
	}

	void NewResourceDialog::update_controls()
	{
		QPushButton* ok_button = m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok);

		const QUuid selected_type = get_selected_type(get_current_selection());
		if (selected_type.isNull() == false)
		{
			ok_button->setEnabled(true);
		}
		else
		{
			ok_button->setEnabled(false);
		}
	}

	QModelIndex NewResourceDialog::get_current_selection() const
	{
		const QModelIndexList selected_indexes = m_ui.typeTreeView->selectionModel()->selectedIndexes();
		if (selected_indexes.isEmpty() == false)
		{
			return selected_indexes.first();
		}

		return QModelIndex();
	}

	QUuid NewResourceDialog::get_selected_type(const QModelIndex& index) const
	{
		if (index.isValid() == true)
		{
			return m_ui.typeTreeView->model()->data(index, static_cast<Qt::ItemDataRole>(Core::TypeTreeDataRole::TYPE_UUID)).toUuid();
		}
		else
		{
			return QUuid();
		}
	}

	void NewResourceDialog::finalize_selection(const QUuid& type_uuid)
	{
		Q_ASSERT_X(type_uuid.isNull() == false, "VadonEditor::UI::NewResourceDialog::finalize_selection", "Must select a valid resource type");

		emit(resource_type_selected(type_uuid));
		accept();
	}

	NewResourceDialogBackend::NewResourceDialogBackend(Core::Application& application, QWidget* dialog_parent, const QString& init_path)
		: m_application(application)
		, m_dialog_parent(dialog_parent)
	{
		// TODO: use init path!
		Q_UNUSED(init_path);

		if (m_dialog_parent == nullptr)
		{
			// If no parent is provided, use main window
			m_dialog_parent = m_application.get_ui_system().get_main_window();
		}

		const QUuid resource_base_type = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ResourceSchema::c_type_uuid);
		NewResourceDialog* dialog = new NewResourceDialog(application, resource_base_type, m_dialog_parent);
		connect(dialog, &NewResourceDialog::resource_type_selected, this, &NewResourceDialogBackend::resource_type_selected);
		
		// Make sure when the dialog is rejected, this object is also cleaned up
		connect(dialog, &NewResourceDialog::rejected, this, &NewResourceDialogBackend::end_workflow);

		dialog->open();
	}

	void NewResourceDialogBackend::resource_type_selected(const QUuid& type_uuid)
	{
		if (is_resource_dialog_excluded_type(type_uuid) == true)
		{
			QMessageBox::warning(m_dialog_parent, "Resource System Warning", "This resource type can only be created via the dedicated workflows!");
			end_workflow();
			return;
		}

		m_new_resource_type = type_uuid;

		SaveAssetDialog* save_dialog = new SaveAssetDialog(m_application, m_dialog_parent);
		connect(save_dialog, &SaveAssetDialog::asset_saved, this, &NewResourceDialogBackend::file_path_selected);
		connect(save_dialog, &SaveAssetDialog::rejected, this, &NewResourceDialogBackend::end_workflow);

		save_dialog->open();
	}

	void NewResourceDialogBackend::file_path_selected(const QString& asset_path)
	{
		create_resource_asset(asset_path);
		end_workflow();
	}

	void NewResourceDialogBackend::end_workflow()
	{
		// End of workflow, clean up object
		QObject::deleteLater();
	}

	void NewResourceDialogBackend::create_resource_asset(const QString& asset_path)
	{
		Q_ASSERT_X(asset_path.isEmpty() == false, "VadonEditor::UI::NewResourceDialogBackend::create_resource_asset", "Invalid path");
		Q_ASSERT_X(m_new_resource_type.isNull() == false, "VadonEditor::UI::NewResourceDialogBackend::create_resource_asset", "Resource type not set");

		// First verify that the asset doesn't already exist
		Core::AssetManager& asset_manager = m_application.get_asset_manager();
		if (asset_manager.find_asset_index_by_path(Core::AssetInfo::get_file_path(asset_path, Core::AssetType::RESOURCE)).isValid() == true)
		{
			QMessageBox::critical(m_dialog_parent, "Asset Manager Error", "Asset file already exists!");
			return;
		}

		// Next try to create the resource
		Model::ResourceSystem& resource_system = m_application.get_model_system().get_resource_system();
		Model::Resource* new_resource = resource_system.create_resource(m_new_resource_type);

		if (new_resource == nullptr)
		{
			QMessageBox::critical(m_dialog_parent, "Resource System Error", "Failed to create resource!");
			return;
		}

		// Create asset
		const int asset_id = resource_system.create_resource_asset(new_resource->get_info().id, asset_path);
		if (asset_id == Core::AssetInfo::c_invalid_file_id)
		{
			QMessageBox::critical(m_dialog_parent, "Resource System Error", "Failed to create resource asset!");
			return;
		}

		// TODO: also print type!
		const QModelIndex asset_index = asset_manager.find_asset_index(asset_id);
		qDebug() << "Resource created at" << asset_manager.get_asset_info(asset_index).path;
	}
}