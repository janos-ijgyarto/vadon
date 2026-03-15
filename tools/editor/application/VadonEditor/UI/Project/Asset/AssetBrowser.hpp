#ifndef VADONEDITOR_UI_PROJECT_ASSET_ASSETBROWSER_HPP
#define VADONEDITOR_UI_PROJECT_ASSET_ASSETBROWSER_HPP
#include <VadonEditor/UI/Project/Asset/ui_AssetBrowserTree.h>
#include <VadonEditor/UI/Project/Asset/ui_SaveAssetDialog.h>
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
	private slots:
		void asset_tree_context_menu_requested(const QPoint& position);

		void new_resource_triggered();

		void selection_changed(const QItemSelection& selected, const QItemSelection& deselected);
		void asset_double_clicked(const QModelIndex& index);
	private:
		QModelIndex get_selected_asset() const;
		Core::AssetInfo get_asset_info(const QModelIndex& index) const;

		void open_asset(const QModelIndex& index);

		Ui::AssetBrowserTree m_ui;
		
		Core::Application* m_application;
	};

	class SaveAssetDialog : public QDialog
	{
		Q_OBJECT
	public:
		SaveAssetDialog(Core::Application& application, QWidget* parent = nullptr);
	signals:
		void asset_saved(const QString& path);
	private slots:
		void asset_name_changed(const QString& text);
		void finalize_asset_save();
	private:
		void update_controls();

		Ui::SaveAssetDialog m_ui;
	};
}
#endif