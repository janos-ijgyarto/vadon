#include <VadonEditor/UI/Project/Asset/AssetBrowser.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/Resource.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/UI/Model/Resource/ResourceDialog.hpp>

#include <QMenu>
#include <QMessageBox>
#include <QPushButton>

namespace VadonEditor::UI
{
	AssetBrowserTree::AssetBrowserTree(QWidget* parent)
		: QWidget(parent)
		, m_application(nullptr)
	{
		m_ui.setupUi(this);
	}

	void AssetBrowserTree::initialize(Core::Application& application)
	{
		Core::AssetManager& asset_manager = application.get_asset_manager();
		m_ui.assetTree->setModel(&asset_manager.get_model());

		connect(m_ui.assetTree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &AssetBrowserTree::selection_changed);

		m_application = &application;
	}

	void AssetBrowserTree::asset_tree_context_menu_requested(const QPoint& position)
	{
#ifndef QT_NO_CONTEXTMENU
		QMenu menu(this);

		QMenu* create_asset_menu = menu.addMenu(tr("Create New Asset"));
		create_asset_menu->addAction(m_ui.actionCreateResource);
		create_asset_menu->addAction(m_ui.actionCreateScene);

		// TODO: add elements if right-click was on existing asset

		menu.exec(m_ui.assetTree->mapToGlobal(position));
#else
		Q_UNUSED(position);
#endif
	}

	void AssetBrowserTree::new_resource_triggered()
	{
		// NOTE: we can fire-and-forget this object, it will clean itself up when the dialog closes
		new NewResourceDialogBackend(*m_application, this); // TODO: use initial path!
	}

	void AssetBrowserTree::new_scene_triggered()
	{
		SaveAssetDialog* save_dialog = new SaveAssetDialog(*m_application, this);
		connect(save_dialog, &SaveAssetDialog::asset_saved, this, &AssetBrowserTree::new_scene_path_selected);

		save_dialog->open();
	}

	void AssetBrowserTree::new_scene_path_selected(const QString& scene_path)
	{
		// FIXME: move all of this to a backend object as well?
		Q_ASSERT_X(scene_path.isEmpty() == false, "VadonEditor::UI::AssetBrowserTree::new_scene_path_selected", "Invalid path");

		// First verify that the asset doesn't already exist
		Core::AssetManager& asset_manager = m_application->get_asset_manager();
		if (asset_manager.find_asset_index_by_path(Core::AssetInfo::get_file_path(scene_path, Core::AssetType::SCENE)).isValid() == true)
		{
			QMessageBox::critical(this, "Asset Manager Error", "Asset file already exists!");
			return;
		}

		// Next try to create the resource
		Model::SceneSystem& scene_system = m_application->get_model_system().get_scene_system();
		Model::Scene* new_scene = scene_system.create_scene();

		if (new_scene == nullptr)
		{
			QMessageBox::critical(this, "Scene System Error", "Failed to create scene!");
			return;
		}

		// Create asset
		const int asset_id = scene_system.create_scene_asset(new_scene->get_resource()->get_info().id, scene_path);
		if (asset_id == Core::AssetInfo::c_invalid_file_id)
		{
			QMessageBox::critical(this, "Scene System Error", "Failed to create scene asset!");
			return;
		}

		// TODO: also print type!
		const QModelIndex asset_index = asset_manager.find_asset_index(asset_id);
		qDebug() << "Scene created at" << asset_manager.get_asset_info(asset_index).path;
	}

	void AssetBrowserTree::selection_changed(const QItemSelection& selected, const QItemSelection& deselected)
	{
		Q_UNUSED(selected);
		Q_UNUSED(deselected);

		// TODO: anything?
	}

	void AssetBrowserTree::asset_double_clicked(const QModelIndex& index)
	{
		open_asset(index);
	}
	
	QModelIndex AssetBrowserTree::get_selected_asset() const
	{
		const QModelIndexList selected_indexes = m_ui.assetTree->selectionModel()->selectedIndexes();
		if (selected_indexes.isEmpty() == false)
		{
			return selected_indexes.first();
		}

		return QModelIndex();
	}

	Core::AssetInfo AssetBrowserTree::get_asset_info(const QModelIndex& index) const
	{
		const Core::AssetManager& asset_manager = m_application->get_asset_manager();
		return asset_manager.get_asset_info(index);
	}

	void AssetBrowserTree::open_asset(const QModelIndex& index)
	{
		Q_ASSERT_X(index.isValid() == true, "VadonEditor::UI::AssetBrowser::open_asset", "Must select a valid asset");

		Core::AssetManager& asset_manager = m_application->get_asset_manager();
		asset_manager.open_asset(index);
	}

	SaveAssetDialog::SaveAssetDialog(Core::Application& application, QWidget* parent)
		: QDialog(parent)
	{
		m_ui.setupUi(this);
		setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);

		m_ui.assetBrowser->initialize(application);
	}

	void SaveAssetDialog::asset_name_changed(const QString& text)
	{
		Q_UNUSED(text);
		update_controls();
	}

	void SaveAssetDialog::finalize_asset_save()
	{
		const QString root_path = m_ui.assetBrowser->get_current_root_path();
		QString asset_path = root_path.isEmpty() ? m_ui.assetBrowser->get_file_name() : QString("%1/%2").arg(root_path).arg(m_ui.assetBrowser->get_file_name());
		emit(asset_saved(asset_path));

		accept();
	}

	void SaveAssetDialog::update_controls()
	{
		QPushButton* save_button = m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Save);
		save_button->setEnabled(m_ui.assetBrowser->get_file_name().isEmpty() == false);
	}

	OpenAssetDialog::OpenAssetDialog(Core::Application& application, QWidget* parent)
		: QDialog(parent)
	{
		m_ui.setupUi(this);
		setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);

		m_ui.assetBrowser->initialize(application);
	}

	void OpenAssetDialog::asset_name_changed(const QString& text)
	{
		Q_UNUSED(text);
		update_controls();
	}

	void OpenAssetDialog::finalize_asset_open()
	{
		const QString root_path = m_ui.assetBrowser->get_current_root_path();
		QString asset_path = root_path.isEmpty() ? m_ui.assetBrowser->get_file_name() : QString("%1/%2").arg(root_path).arg(m_ui.assetBrowser->get_file_name());
		emit(asset_opened(asset_path));

		accept();
	}

	void OpenAssetDialog::update_controls()
	{
		QPushButton* open_button = m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Open);
		open_button->setEnabled(m_ui.assetBrowser->get_file_name().isEmpty() == false);
	}
}