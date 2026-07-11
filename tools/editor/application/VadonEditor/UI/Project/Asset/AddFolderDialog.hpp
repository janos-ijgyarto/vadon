#ifndef VADONEDITOR_UI_PROJECT_ASSET_ADDFOLDERDIALOG_HPP
#define VADONEDITOR_UI_PROJECT_ASSET_ADDFOLDERDIALOG_HPP
#include <VadonEditor/UI/Project/Asset/ui_AddFolderDialog.h>
#include <QDialog>
#include <QModelIndex>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::UI
{
	class AddFolderDialog : public QDialog
	{
		Q_OBJECT
	public:
		AddFolderDialog(Core::Application& application, QModelIndex parent_asset, QWidget* parent = nullptr);
	private slots:
		void folder_name_accepted();
		void folder_name_changed(const QString& text);
	private:
		void update_controls();

		Core::Application& m_application;
		QModelIndex m_parent_asset;

		Ui::AddFolderDialog m_ui;
	};
}
#endif