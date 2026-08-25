#ifndef VADONEDITOR_UI_PROJECT_ASSET_ASSETDIALOG_HPP
#define VADONEDITOR_UI_PROJECT_ASSET_ASSETDIALOG_HPP
#include <VadonEditor/UI/Project/Asset/ui_OpenAssetDialog.h>
#include <VadonEditor/UI/Project/Asset/ui_SaveAssetDialog.h>
namespace VadonEditor::UI
{
	class SaveAssetDialog : public QDialog
	{
		Q_OBJECT
	public:
		SaveAssetDialog(Core::Application& application, QWidget* parent = nullptr, const QModelIndex& root_asset = QModelIndex());

		QString get_asset_path() const;
	signals:
		void asset_saved(const QString& path);
	protected:
		virtual void internal_finalize_asset_save();

		Core::Application& m_application;
	private slots:
		void asset_name_changed(const QString& text);
		void finalize_asset_save() { internal_finalize_asset_save(); }
	private:
		void update_controls();

		Ui::SaveAssetDialog m_ui;
	};

	class OpenAssetDialog : public QDialog
	{
		Q_OBJECT
	public:
		OpenAssetDialog(Core::Application& application, QWidget* parent = nullptr);
	signals:
		void asset_opened(const QString& path);
	private slots:
		void asset_name_changed(const QString& text);
		void finalize_asset_open();
	private:
		void update_controls();

		Ui::OpenAssetDialog m_ui;
	};
}
#endif