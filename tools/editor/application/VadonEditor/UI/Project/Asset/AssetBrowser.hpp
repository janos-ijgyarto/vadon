#ifndef VADONEDITOR_UI_PROJECT_ASSET_ASSETBROWSER_HPP
#define VADONEDITOR_UI_PROJECT_ASSET_ASSETBROWSER_HPP
#include <VadonEditor/UI/Project/Asset/ui_AssetBrowser.h>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::UI
{
	class AssetBrowser : public QWidget
	{
		Q_OBJECT
	public:
		AssetBrowser(QWidget* parent = nullptr);

		void initialize(Core::Application& application);
	private slots:
		void asset_tree_context_menu_requested(const QPoint& position);

		void new_resource_triggered();
	private:
		Ui::AssetBrowser m_ui;
	};
}
#endif