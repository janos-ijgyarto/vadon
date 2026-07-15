#ifndef VADONEDITOR_UI_PROJECT_ASSET_ASSETBROWSER_HPP
#define VADONEDITOR_UI_PROJECT_ASSET_ASSETBROWSER_HPP
#include <VadonEditor/UI/Project/Asset/ui_AssetBrowserTree.h>
#include <QUuid>
namespace VadonEditor::Core
{
	class Application;
	struct AssetInfo;
}
namespace VadonEditor::UI
{
	class AssetBrowserTree : public QWidget
	{
		Q_OBJECT
	public:
		AssetBrowserTree(QWidget* parent = nullptr);

		void initialize(Core::Application& application); 
	protected:
		bool eventFilter(QObject* obj, QEvent* event) override;
	private slots:
		void asset_tree_context_menu_requested(const QPoint& position);

		void add_folder_triggered();
		void new_resource_triggered();
		void new_scene_triggered();

		void selection_changed(const QItemSelection& selected, const QItemSelection& deselected);
		void asset_double_clicked(const QModelIndex& index);
	private:
		QModelIndex get_selected_asset() const;
		Core::AssetInfo get_asset_info(const QModelIndex& index) const;

		void open_asset(const QModelIndex& index);

		Ui::AssetBrowserTree m_ui;
		
		Core::Application* m_application;
	};
}
#endif