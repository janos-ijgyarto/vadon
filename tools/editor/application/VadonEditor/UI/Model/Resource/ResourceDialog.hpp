#ifndef VADONEDITOR_UI_MODEL_RESOURCE_RESOURCEDIALOG_HPP
#define VADONEDITOR_UI_MODEL_RESOURCE_RESOURCEDIALOG_HPP
#include <VadonEditor/UI/Model/Resource/ui_NewResourceDialog.h>
#include <QDialog>
#include <QUuid>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::UI
{
	class NewResourceDialog : public QDialog
	{
		Q_OBJECT
	public:
		NewResourceDialog(Core::Application& application, const QUuid& base_type, QWidget* parent = nullptr);
	signals:
		void resource_type_selected(const QUuid& type_uuid);
	private slots:
		void type_double_clicked(const QModelIndex& index);

		void selection_changed(const QItemSelection& selected, const QItemSelection& deselected);
		void selection_accepted();
	private:
		void update_controls();

		QModelIndex get_current_selection() const;
		QUuid get_selected_type(const QModelIndex& index) const;

		void finalize_selection(const QUuid& type_uuid);

		Ui::NewResourceDialog m_ui;
	};

	// NOTE: this is a utility object for NewResourceDialog which encapsulates
	// the logic of creating the new resource asset based on the user's selection
	// This helps keep the implementation of the UI objects (including the dialog) simpler
	class NewResourceDialogBackend : public QObject
	{
		Q_OBJECT
	public:
		NewResourceDialogBackend(Core::Application& application, QWidget* dialog_parent, const QString& init_path = "");
	private slots:
		void resource_type_selected(const QUuid& type_uuid);
		void file_path_selected(const QString& asset_path);

		void end_workflow();
	private:
		void create_resource_asset(const QString& asset_path);

		Core::Application& m_application;

		QWidget* m_dialog_parent;
		QUuid m_new_resource_type;
	};
}
#endif