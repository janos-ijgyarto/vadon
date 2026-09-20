#include <VadonEditor/UI/Project/Asset/AssetBrowser.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/Resource/Resource.hpp>

#include <VadonEditor/UI/Model/Resource/ResourceDialog.hpp>
#include <VadonEditor/UI/Model/Scene/SceneDialog.hpp>

#include <VadonEditor/UI/Project/Asset/AddFolderDialog.hpp>

#include <QMenu>
#include <QMouseEvent>

namespace VadonEditor::UI
{
	AssetBrowserTree::AssetBrowserTree(QWidget* parent)
		: QWidget(parent)
		, m_application(nullptr)
	{
		m_ui.setupUi(this);

		m_ui.assetTree->viewport()->installEventFilter(this);
	}

	void AssetBrowserTree::initialize(Core::Application& application)
	{
		Core::AssetManager& asset_manager = application.get_asset_manager();
		m_ui.assetTree->setModel(&asset_manager.get_model());

		connect(m_ui.assetTree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &AssetBrowserTree::selection_changed);

		m_application = &application;
	}

	bool AssetBrowserTree::eventFilter(QObject* obj, QEvent* event)
	{
		if (obj == m_ui.assetTree->viewport())
		{
			if (event->type() == QEvent::MouseButtonRelease)
			{
				QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
				if (m_ui.assetTree->indexAt(mouse_event->pos()).isValid() == false)
				{
					m_ui.assetTree->selectionModel()->clearSelection();
				}
			}
		}

		// Standard event processing
		return QObject::eventFilter(obj, event);
	}

	void AssetBrowserTree::asset_tree_context_menu_requested(const QPoint& position)
	{
#ifndef QT_NO_CONTEXTMENU
		QMenu menu(this);

		menu.addAction(m_ui.actionNewFolder);

		QMenu* create_asset_menu = menu.addMenu(tr("Create New Asset"));
		create_asset_menu->addAction(m_ui.actionCreateResource);
		create_asset_menu->addAction(m_ui.actionCreateScene);

		QModelIndex item_index = m_ui.assetTree->indexAt(position);
		if (item_index.isValid() == true)
		{
			Core::AssetManager& asset_manager = m_application->get_asset_manager();
			const Core::AssetInfo asset_info = asset_manager.get_asset_info(item_index);
			Q_ASSERT_X(asset_info.is_valid() == true, "VadonEditor::UI::AssetBrowserTree::asset_tree_context_menu_requested", "Invalid asset");

			switch (asset_info.type)
			{
			case Core::AssetType::SCENE:
				create_asset_menu->addAction(m_ui.actionCreateInheritedScene);
				break;
				// TODO: options for other asset types?
			}
		}

		menu.exec(m_ui.assetTree->mapToGlobal(position));
#else
		Q_UNUSED(position);
#endif
	}

	void AssetBrowserTree::add_folder_triggered()
	{
		AddFolderDialog* add_folder_dialog = new AddFolderDialog(*m_application, get_selected_asset(), this);
		add_folder_dialog->open();
	}

	void AssetBrowserTree::new_resource_triggered()
	{
		NewResourceDialog* new_resource_dialog = new NewResourceDialog(*m_application, Model::Resource::get_base_resource_type(), get_selected_asset(), this);
		new_resource_dialog->open();
	}

	void AssetBrowserTree::new_scene_triggered()
	{
		NewSceneDialog* scene_dialog = new NewSceneDialog(*m_application, this, get_selected_asset());
		scene_dialog->open();
	}

	void AssetBrowserTree::inherited_scene_triggered()
	{
		InheritedSceneDialog* inherited_dialog = new InheritedSceneDialog(*m_application, get_selected_asset(), this);
		inherited_dialog->open();
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
}