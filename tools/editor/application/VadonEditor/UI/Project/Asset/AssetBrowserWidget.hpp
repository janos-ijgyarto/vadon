#ifndef VADONEDITOR_UI_PROJECT_ASSET_ASSETBROWSERWIDGET_HPP
#define VADONEDITOR_UI_PROJECT_ASSET_ASSETBROWSERWIDGET_HPP
#include <VadonEditor/UI/Project/Asset/ui_AssetBrowserWidget.h>
namespace VadonEditor::Core
{
	class Application;
	struct AssetInfo;
}
namespace VadonEditor::UI
{
	class AssetBrowserWidget : public QWidget
	{
		Q_OBJECT
	public:
		AssetBrowserWidget(QWidget* parent = nullptr);

		void initialize(Core::Application& application, const QModelIndex& root_asset = QModelIndex());

		QModelIndex get_selected_asset() const;
		Core::AssetInfo get_asset_info(const QModelIndex& index) const;

		QString get_file_name() const { return m_ui.fileNameLineEdit->text(); }

		QString get_current_root_path() const;
	signals:
		void asset_selected(const QModelIndex& index);
		void asset_opened(const QModelIndex& index);

		void file_name_changed(const QString& text);
	private slots:
		void prev_clicked();
		void next_clicked();
		void up_clicked();

		void selection_changed(const QItemSelection& selected, const QItemSelection& deselected);
		void asset_double_clicked(const QModelIndex& index);

		void name_line_edit_changed(const QString& text);
	private:
		void set_root(const QModelIndex& root_index);
		void update_controls();

		Ui::AssetBrowserWidget m_ui;

		Core::Application* m_application;
	};
}
#endif