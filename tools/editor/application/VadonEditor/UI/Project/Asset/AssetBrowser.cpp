#include <VadonEditor/UI/Project/Asset/AssetBrowser.hpp>

#include <VadonEditor/Core/Application.hpp>

#include <VadonEditor/Core/Asset/AssetManager.hpp>

#include <QMenu>

namespace VadonEditor::UI
{
	AssetBrowser::AssetBrowser(QWidget* parent)
		: QWidget(parent)
	{
		m_ui.setupUi(this);
	}

	void AssetBrowser::initialize(Core::Application& application)
	{
		m_ui.assetTree->setModel(&application.get_asset_manager().get_model());
	}

	void AssetBrowser::asset_tree_context_menu_requested(const QPoint& position)
	{
#ifndef QT_NO_CONTEXTMENU
		QMenu menu(this);

		QMenu* create_asset_menu = menu.addMenu(tr("Create New Asset"));
		create_asset_menu->addAction(m_ui.actionCreateResource);

		// TODO: add elements if right-click was on existing asset

		menu.exec(m_ui.assetTree->mapToGlobal(position));
#else
		Q_UNUSED(position);
#endif
	}

	void AssetBrowser::new_resource_triggered()
	{
		qDebug() << "New resource requested!";
	}
}