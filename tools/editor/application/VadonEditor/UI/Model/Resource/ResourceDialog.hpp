#ifndef VADONEDITOR_UI_MODEL_RESOURCE_RESOURCEDIALOG_HPP
#define VADONEDITOR_UI_MODEL_RESOURCE_RESOURCEDIALOG_HPP
#include <VadonEditor/UI/Model/Resource/ui_NewResourceDialog.h>
#include <VadonEditor/UI/Model/Resource/ui_SelectResourceDialog.h>
#include <QDialog>
#include <QSortFilterProxyModel>
#include <QUuid>
namespace VadonEditor::Core
{
	class Application;
	class TypeFilterModel;
}
namespace VadonEditor::Model
{
	struct ResourceInfo;
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
		void filter_text_changed(const QString& text);

		void selection_changed(const QItemSelection& selected, const QItemSelection& deselected);
		void selection_accepted();
	private:
		void update_controls();

		QModelIndex get_current_selection() const;
		QUuid get_selected_type(const QModelIndex& index) const;

		void finalize_selection(const QUuid& type_uuid);

		Ui::NewResourceDialog m_ui;

		Core::TypeFilterModel* m_filter_model;
	};

	// NOTE: this is a utility object for NewResourceDialog which encapsulates
	// the logic of creating the new resource asset based on the user's selection
	// This helps keep the implementation of the UI objects (including the dialog) simpler
	class NewResourceDialogBackend : public QObject
	{
		Q_OBJECT
	public:
		NewResourceDialogBackend(Core::Application& application, QWidget* dialog_parent, const QModelIndex& root_asset = QModelIndex());
	private slots:
		void resource_type_selected(const QUuid& type_uuid);
		void file_path_selected(const QString& asset_path);

		void end_workflow();
	private:
		void create_resource_asset(const QString& asset_path);

		Core::Application& m_application;

		QWidget* m_dialog_parent;
		QUuid m_new_resource_type;
		QModelIndex m_root_asset;
	};

	class ResourceAssetFilterModel : public QSortFilterProxyModel
	{
		Q_OBJECT
	public:
		ResourceAssetFilterModel(Core::Application& application, const QUuid& resource_type, QObject* parent = nullptr);

		const QUuid& get_resource_type() const { return m_resource_type; }
	protected:
		bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
	private:
		Core::Application& m_application;
		QUuid m_resource_type;
	};

	class SelectResourceDialog : public QDialog
	{
		Q_OBJECT
	public:
		SelectResourceDialog(Core::Application& application, const QUuid& resource_type, QWidget* dialog_parent = nullptr);
	signals:
		void resource_asset_selected(const QUuid& resource_id);
	private slots:
		void filter_text_changed(const QString& text);
		void tree_item_clicked(const QModelIndex& index);
		void tree_item_double_clicked(const QModelIndex& index);
	private:
		void update_buttons();
		bool is_compatible_item(const Model::ResourceInfo& resource_info) const;
		Model::ResourceInfo get_resource_info(const QModelIndex& index) const;

		Ui::SelectResourceDialog m_ui;

		Core::Application& m_application;
		ResourceAssetFilterModel m_filter_model;
		QModelIndex m_selection;
	};
}
#endif