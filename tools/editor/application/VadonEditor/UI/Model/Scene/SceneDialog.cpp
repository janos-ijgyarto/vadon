#include <VadonEditor/UI/Model/Scene/SceneDialog.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/Resource.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <QMessageBox>

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

		Q_ASSERT_X(new_scene_path.isEmpty() == false, "VadonEditor::UI::NewSceneDialog::internal_finalize_asset_save", "Invalid path");

		// First verify that the asset doesn't already exist
		Core::AssetManager& asset_manager = m_application.get_asset_manager();
		if (asset_manager.find_asset_index_by_path(Core::AssetInfo::get_file_path(new_scene_path, Core::AssetType::SCENE)).isValid() == true)
		{
			QMessageBox::critical(this, "Asset Manager Error", "Asset file already exists!");
			return;
		}

		// Next try to create the resource
		Model::SceneSystem& scene_system = m_application.get_model_system().get_scene_system();
		Model::Scene* new_scene = scene_system.create_scene();

		if (new_scene == nullptr)
		{
			QMessageBox::critical(this, "Scene System Error", "Failed to create scene!");
			return;
		}

		// Create asset
		const int asset_id = scene_system.create_scene_asset(new_scene->get_resource()->get_info().id, new_scene_path);
		if (asset_id == Core::AssetInfo::c_invalid_file_id)
		{
			QMessageBox::critical(this, "Scene System Error", "Failed to create scene asset!");
			return;
		}

		// TODO: also print type!
		const QModelIndex asset_index = asset_manager.find_asset_index(asset_id);
		qDebug() << "Scene created at" << asset_manager.get_asset_info(asset_index).path;

		// Use base method to accept dialog
		SaveAssetDialog::internal_finalize_asset_save();
	}
}