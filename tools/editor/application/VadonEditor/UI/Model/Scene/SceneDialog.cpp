#include <VadonEditor/UI/Model/Scene/SceneDialog.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/Resource.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/UI/Model/Resource/ResourceDialog.hpp>

#include <QMessageBox>

namespace
{
	bool scene_dialog_create_new_scene(VadonEditor::Core::Application& application, QWidget* parent_widget, const QString& scene_asset_path, const QUuid& base_scene_id = QUuid{})
	{
		Q_ASSERT_X(scene_asset_path.isEmpty() == false, "scene_dialog_create_new_scene", "Invalid path");

		// First verify that the asset doesn't already exist
		VadonEditor::Core::AssetManager& asset_manager = application.get_asset_manager();
		if (asset_manager.find_asset_index_by_path(VadonEditor::Core::AssetInfo::get_file_path(scene_asset_path, VadonEditor::Core::AssetType::SCENE)).isValid() == true)
		{
			QMessageBox::critical(parent_widget, "Asset Manager Error", "Asset file already exists!");
			return false;
		}

		// Next try to create the resource
		VadonEditor::Model::SceneSystem& scene_system = application.get_model_system().get_scene_system();
		VadonEditor::Model::Scene* new_scene = scene_system.create_scene(base_scene_id);

		if (new_scene == nullptr)
		{
			QMessageBox::critical(parent_widget, "Scene System Error", "Failed to create scene!");
			return false;
		}

		// Create asset
		const int asset_id = scene_system.create_scene_asset(new_scene->get_resource()->get_info().id, scene_asset_path);
		if (asset_id == VadonEditor::Core::AssetInfo::c_invalid_file_id)
		{
			QMessageBox::critical(parent_widget, "Scene System Error", "Failed to create scene asset!");
			return false;
		}

		// TODO: also print type!
		const QModelIndex asset_index = asset_manager.find_asset_index(asset_id);
		qDebug() << "Scene created at" << asset_manager.get_asset_info(asset_index).path;

		return true;
	}
}

namespace VadonEditor::UI
{
	NewSceneDialog::NewSceneDialog(Core::Application& application, QWidget* parent, const QModelIndex& root_asset)
		: SaveAssetDialog(application, parent, root_asset)
	{
		setWindowTitle("Save Scene As");
	}

	void NewSceneDialog::internal_finalize_asset_save()
	{
		const QString new_scene_path = get_asset_path();

		if (scene_dialog_create_new_scene(m_application, this, new_scene_path) == true)
		{
			// Use base method to accept dialog
			SaveAssetDialog::internal_finalize_asset_save();
		}
	}

	InheritedSceneDialog::InheritedSceneDialog(Core::Application& application, const QModelIndex& base_scene, QWidget* parent)
		: QDialog(parent)
		, m_application(application)
	{
		m_ui.setupUi(this);
		setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);

		if (base_scene.isValid() == true)
		{
			const Core::AssetInfo& asset_info = m_application.get_asset_manager().get_asset_info(base_scene);
			if (asset_info.type == Core::AssetType::SCENE)
			{
				const Model::ResourceInfo resource_info = m_application.get_model_system().get_resource_system().resource_info_by_asset_id(asset_info.id);
				base_scene_selected(resource_info.id);
			}
			else
			{
				// TODO: log error!
			}
		}

		update_controls();
	}

	void InheritedSceneDialog::accept()
	{
		// FIXME: find a way to deduplicate the
		Q_ASSERT_X(Utilities::is_uuid_valid(m_base_scene) == true, "VadonEditor::UI::InheritedSceneDialog::accept", "Invalid base scene");

		if (scene_dialog_create_new_scene(m_application, this, m_inherited_scene_path, m_base_scene) == true)
		{
			// Use base method to accept dialog
			QDialog::accept();
		}
	}

	void InheritedSceneDialog::browse_base_scene_clicked()
	{
		SelectResourceDialog* select_dialog = new SelectResourceDialog(m_application, Model::Scene::get_scene_type_uuid(), this);
		connect(select_dialog, &SelectResourceDialog::resource_asset_selected, this, &InheritedSceneDialog::base_scene_selected);

		select_dialog->open();
	}

	void InheritedSceneDialog::base_scene_selected(const QUuid& scene_uuid)
	{
		const int scene_asset_id = m_application.get_model_system().get_scene_system().find_scene_asset(scene_uuid);

		Core::AssetManager& asset_manager = m_application.get_asset_manager();
		const QModelIndex asset_index = asset_manager.find_asset_index(scene_asset_id);
		if (asset_index.isValid() == false)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::InheritedSceneDialog::base_scene_selected", "Cannot find scene asset");
			return;
		}

		const Core::AssetInfo& scene_asset_info = asset_manager.get_asset_info(asset_index);
		m_ui.baseScenePath->setText(scene_asset_info.path);

		m_base_scene = scene_uuid;

		// Clear path, user should select it again
		m_inherited_scene_path.clear();

		update_controls();
	}

	void InheritedSceneDialog::browse_inherited_scene_clicked()
	{
		SaveAssetDialog* save_dialog = new SaveAssetDialog(m_application, this);
		connect(save_dialog, &SaveAssetDialog::asset_saved, this, &InheritedSceneDialog::inherited_scene_path_selected);

		save_dialog->setWindowTitle("Select Inherited Scene Path");

		save_dialog->open();
	}

	void InheritedSceneDialog::inherited_scene_path_selected(const QString& asset_path)
	{
		m_inherited_scene_path = asset_path;
		update_controls();
	}

	void InheritedSceneDialog::update_controls()
	{
		const bool valid_base_scene = Utilities::is_uuid_valid(m_base_scene);
		QPushButton* ok_button = m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok);

		if (m_inherited_scene_path.isEmpty() == false)
		{
			m_ui.inheritedScenePath->setText(Core::AssetInfo::get_file_path(m_inherited_scene_path, Core::AssetType::SCENE));			
		}
		else
		{
			m_ui.inheritedScenePath->clear();
		}

		if (valid_base_scene == false)
		{
			m_ui.inheritedSceneBrowseButton->setEnabled(false);
			ok_button->setEnabled(false);
			return;
		}

		m_ui.inheritedSceneBrowseButton->setEnabled(true);
		if (m_inherited_scene_path.isEmpty() == true)
		{
			ok_button->setEnabled(false);
			return;
		}

		ok_button->setEnabled(true);
	}
}